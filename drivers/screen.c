/*
 * screen.c
 *
 *  Created on: Dec 3, 2018
 *      Author: Beau
 */
#include "screen.h"
#include "ss_display_driver.h"
#include "mapping.h"
#include "time.h"
#include "MK66F18.h"
#include "fsl_sd_disk.h"
#include <string.h>
#include <stdlib.h>

extern edma_handle_t * s_EDMAHandle;
uint8_t ones = 0xFF;
uint8_t screen[BITS_PER_LED*LEDS_PER_STRIP];
uint32_t update_in_progress = 0;
uint32_t update_time = 0;
uint32_t update_period_ms=0;

DMA_Type * p_dma_regs = EDMA_1_DMA_BASEADDR;
FTM_Type * p_ftm_regs = WS28XX_BIT_TIME_PERIPHERAL;
DMAMUX_Type * p_dma_mux_regs = DMAMUX_BASE;
PORT_Type * p_porta_regs = PORTC_BASE;
LPTMR_Type * p_lptimer_regs = LPTMR0_BASE;

screenMapping map;
static x_y map_mem[MAP_COLUMNS][MAP_ROWS];

song_effects * pSongs[MAX_NUM_SONGS];
FATFS fs;

void WS28XX_BIT_TIME_IRQHANDLER(void)
{
	// read FTM STATUS REGISTERS FMS, SC, AND STATUS
	// SC: b7 TOF 	- TIMER OVERFLOW FLAG
	//				- CLEARED BY READING REG, THEN WRITING A 0 TO TOF bit
	// STATUS: A MIRROR copy of every CHnF bit in CnSC, can be cleared by reading then writing a 0 here
	// FMS - FAULT MODE STATUS - don't do
	GPIOD->PSOR = 1;
	uint32_t overflow = FTM2->SC;
	uint32_t chnf __attribute__((unused)) = FTM2->STATUS;
	FTM2->SC = (overflow & 0x7F);	//ack overflow
	FTM2->STATUS = 0; 				//ack any channel flag (match)
	GPIOD->PCOR = 1;
}

void DMA2_DMA18_IRQHandler(void)
{
	//set flag to signal that dma has finished updating the screen
	update_time = LPTMR_GetCurrentTimerCount(LPTMR0);
	update_in_progress = 0;

	//acknowledge interrupt
	p_dma_regs->CINT = 2;	//channel 2
}

void init_screen_modules(void)
{
	time_init();

	// manually configure everything needed for driving the leds
	// should be a combination of fta, dma mux, dma, and port control to signal interrupt/dma request
	//reconfigure FTM BEFORE turning on the clock
	p_ftm_regs->CONTROLS[0].CnSC = 0x69;
	p_ftm_regs->CONTROLS[1].CnSC = 0x69;

	//Trigger dma requests on output of either channel of ftm. ftm ch 0 controlls pta10
	//ensure that it does and that it's port is set to raise dma requests
	//port control and interrupts (PORT)
	PORT_SetPinInterruptConfig(PORTA, 10U, kPORT_DMARisingEdge);

	//Clear the ENABLE AND TRIGGER ON DMA MUX CHANNEL
	p_dma_mux_regs->CHCFG[0] = 0;
	p_dma_mux_regs->CHCFG[1] = 0;
	p_dma_mux_regs->CHCFG[2] = 0;

	//CONFIGURE THE DMA, MAY ENABLE DMA CHANNEL
		//1. write to cr is a configuration other than the default is desired
		p_dma_regs->CR = 0x100;	//Fixed Priority Arbitration with, Group 0 having higher priority than Group 1
		//2. WRITE THE CHANNEL PRIORITY LEVELS TO THE DCHPRIN registers
		p_dma_regs->DCHPRI0 = DMA_DCHPRI0_ECP(0) 	| 		//allow for other higher priorities to pause you
					DMA_DCHPRI0_DPA(1) 				|		//can suspend lower priority channel in dma
					DMA_DCHPRI0_CHPRI(15);					//HIGHEST PRIORITY 15
		p_dma_regs->DCHPRI15 = DMA_DCHPRI0_ECP(0) 	|		//within group, priorities must be unique or will cause an error
					DMA_DCHPRI0_DPA(0) 				|		//switching previous priority with previous owner of priority 15
					DMA_DCHPRI0_CHPRI(0);
		p_dma_regs->DCHPRI1 = DMA_DCHPRI0_ECP(0) 	|	//channel 1 priority 14 (FTM 2 CH0 Match)
					DMA_DCHPRI0_DPA(1) 				|
					DMA_DCHPRI0_CHPRI(14);
		p_dma_regs->DCHPRI14 = DMA_DCHPRI0_ECP(0) 	|	//channel 14 priority 1 (SWAPPED)
					DMA_DCHPRI0_DPA(0) 				|
					DMA_DCHPRI0_CHPRI(1);
		p_dma_regs->DCHPRI2 = DMA_DCHPRI0_ECP(0) 	|	//channel 2 priority 13 (FTM 2 CH1 Match)
					DMA_DCHPRI0_DPA(1) 				|
					DMA_DCHPRI0_CHPRI(13);
		p_dma_regs->DCHPRI13 = DMA_DCHPRI0_ECP(0) 	|	//channel 13 priority 2 (SWAPPED)
					DMA_DCHPRI0_DPA(0) 				|
					DMA_DCHPRI0_CHPRI(2);

		//3. Enable Error interrupts in the EEI reg if desired
		//NONE
		//4. write the 32 byte TCD for each channel that may request service
			uint32_t numPulses = BITS_PER_LED*LEDS_PER_STRIP;
			//TCD CH 0 - rise all outputs at beginning of period/overflow
			p_dma_regs->TCD[0].ATTR = 0; // same as: DMA_ATTR_SMOD(0) | DMA_ATTR_SSIZE(0) | DMA_ATTR_DMOD(0) | DMA_ATTR_DSIZE(0);
			p_dma_regs->TCD[0].SADDR = (uint32_t) &ones;
			p_dma_regs->TCD[0].SOFF = 0;
			p_dma_regs->TCD[0].NBYTES_MLNO = 1;
			p_dma_regs->TCD[0].SLAST = 0;
			p_dma_regs->TCD[0].DADDR = (uint32_t) &(GPIOD->PSOR);
			p_dma_regs->TCD[0].DOFF = 0;
			p_dma_regs->TCD[0].DLAST_SGA = 0;
			p_dma_regs->TCD[0].CITER_ELINKNO = numPulses; //24 PULSES PER LED
			p_dma_regs->TCD[0].BITER_ELINKNO = numPulses;
			p_dma_regs->TCD[0].CSR = DMA_CSR_DREQ(1); // DREQ

			//TCD CH 1 - set DATA outputs at FTM2 CH0 Match
			p_dma_regs->TCD[1].ATTR = 0; // same as: DMA_ATTR_SMOD(0) | DMA_ATTR_SSIZE(0) | DMA_ATTR_DMOD(0) | DMA_ATTR_DSIZE(0);
			p_dma_regs->TCD[1].SADDR = (uint32_t) &screen;
			p_dma_regs->TCD[1].SOFF = 1;
			p_dma_regs->TCD[1].NBYTES_MLNO = 1;
			p_dma_regs->TCD[1].SLAST = -numPulses;
			p_dma_regs->TCD[1].DADDR = (uint32_t) &(GPIOD->PDOR);
			p_dma_regs->TCD[1].DOFF = 0;
			p_dma_regs->TCD[1].DLAST_SGA = 0;
			p_dma_regs->TCD[1].CITER_ELINKNO = numPulses; //24 PULSES PER LED
			p_dma_regs->TCD[1].BITER_ELINKNO = numPulses;
			p_dma_regs->TCD[1].CSR = DMA_CSR_DREQ(1); // DREQ
			//TCD CH 2
			p_dma_regs->TCD[2].ATTR = 0; // same as: DMA_ATTR_SMOD(0) | DMA_ATTR_SSIZE(0) | DMA_ATTR_DMOD(0) | DMA_ATTR_DSIZE(0);
			p_dma_regs->TCD[2].SADDR = (uint32_t) &ones;
			p_dma_regs->TCD[2].SOFF = 0;
			p_dma_regs->TCD[2].NBYTES_MLNO = 1;
			p_dma_regs->TCD[2].SLAST = 0;
			p_dma_regs->TCD[2].DADDR = (uint32_t) &(GPIOD->PCOR);
			p_dma_regs->TCD[2].DOFF = 0;
			p_dma_regs->TCD[2].DLAST_SGA = 0;
			p_dma_regs->TCD[2].CITER_ELINKNO = numPulses; //24 PULSES PER LED
			p_dma_regs->TCD[2].BITER_ELINKNO = numPulses;
			p_dma_regs->TCD[2].CSR = DMA_CSR_DREQ(1) | DMA_CSR_INTMAJOR(1); // DREQ & INTTERUPT ON MAJOR LOOP COMPLETION

		//5. Enable any hardware service request via the ERQH AND ERQL registers
		//p_dma_regs->EARS = 1; 	//enabling asynchronous dma request in stop mode for channel 0
		p_dma_regs->SERQ = 0; 	//EN request for each signal, muxed with dma request from other modules (port A)
		p_dma_regs->SERQ = 1;	//en request for ftm channel 0 match
		p_dma_regs->SERQ = 2;	//en request for ftm channel 1 match
		//6. Request Channel service via either
			//software: setting the TCDn_CSR[START],... MAY DO JUST FOR CONFIGURATION CHECK
			//Hardware: slave device asserting its eDMA eripheral request signal

	//ENABLE AND SELECT CHANNEL SOURCE IN DMA MUX
	p_dma_mux_regs->CHCFG[0] = DMAMUX_CHCFG_ENBL(1) | DMAMUX_CHCFG_SOURCE(49);	//49 - Port Control Module - Port A
	p_dma_mux_regs->CHCFG[1] = DMAMUX_CHCFG_ENBL(1) | DMAMUX_CHCFG_SOURCE(30);	//30 - FTM2 CH 0 Match
	p_dma_mux_regs->CHCFG[2] = DMAMUX_CHCFG_ENBL(1) | DMAMUX_CHCFG_SOURCE(31);	//30 - FTM2 CH 1 Match

	//clear the leds by writing 0 to all of them
	//then continue main code
	update_in_progress = 1;
	FTM_StartTimer(WS28XX_BIT_TIME_PERIPHERAL, kFTM_SystemClock);
	while (update_in_progress);
}

void init_mapping(void)
{
	for(int map_strip_idx=0; map_strip_idx < MAP_RECT_STRIPS; map_strip_idx++)
	{
		int32_t delta_x = (int32_t)(xy_strip_mapping[map_strip_idx].end_x - xy_strip_mapping[map_strip_idx].begin_x);
		int32_t delta_y = (int32_t)(xy_strip_mapping[map_strip_idx].end_y - xy_strip_mapping[map_strip_idx].begin_y);
		if(delta_x == 0)
		{
			int delta_inc;
			if(delta_y > 0)
			{
				delta_inc = 1;
			}else{
				delta_inc = -1;
			}

			uint32_t ledIdxOnStrip = xy_strip_mapping[map_strip_idx].strip_begin_led_idx;
			for(int y = xy_strip_mapping[map_strip_idx].begin_y; y != xy_strip_mapping[map_strip_idx].end_y; y+=delta_inc )
			{
				map_mem[xy_strip_mapping[map_strip_idx].begin_x][y].valid = 1;
				map_mem[xy_strip_mapping[map_strip_idx].begin_x][y].led_index = ledIdxOnStrip++;
				map_mem[xy_strip_mapping[map_strip_idx].begin_x][y].strip_index = xy_strip_mapping[map_strip_idx].strip_idx;
			}

			//do one more for the end point
			map_mem[xy_strip_mapping[map_strip_idx].end_x][xy_strip_mapping[map_strip_idx].end_y].valid = 1;
			map_mem[xy_strip_mapping[map_strip_idx].end_x][xy_strip_mapping[map_strip_idx].end_y].led_index = ledIdxOnStrip++;
			map_mem[xy_strip_mapping[map_strip_idx].end_x][xy_strip_mapping[map_strip_idx].end_y].strip_index = xy_strip_mapping[map_strip_idx].strip_idx;

		}else
		{
			//do later, mapping if strips are layed out horizantal instead of vertical
		}
	}
}

void init_sdcard(void)
{
	/*sdhc_config_t sd_conf;
	sd_conf.cardDetectDat3 = true;
	sd_conf.endianMode = kSDHC_EndianModeLittle;
	sd_conf.dmaMode = kSDHC_DmaModeAdma2;
	sd_conf.readWatermarkLevel = 128U;
	sd_conf.writeWatermarkLevel = 128U;
	SDHC_Init(SDHC, &sd_conf);
	*/

	//must initialize the sd host prior to engaging with sd module through ff
    /* Save host information. */
    //g_sd.host.base = SD_HOST_BASEADDR;
    //g_sd.host.sourceClock_Hz = SD_CLOCK_25MHZ;//SD_HOST_CLK_FREQ;
    /* SD host init function */
	/*
    if (SD_HostInit(&g_sd) != kStatus_Success)
    {
        printf("\r\nSD host init fail\r\n");
    }*/

	FRESULT fr;
	char line[100];
	fr = f_mount(&fs, "4:", 1);
	if(fr != FR_OK)
	{
		while(fr != FR_OK)
		{
			ss_set_err_code(SS_ERR_CODE_NO_SD);

			//wait some time
			wait_ms_blocking(250);

			fr = f_mount(&fs, "4:", 1);
		}
	}
	f_chdrive("4:/");

	//fr = f_open(&fil, "arctic.out", FA_READ);

	//f_gets(line,sizeof(line), &fil);
	//printf(line);

	//f_close(&fil);

}

int32_t screen_init(song_conf * pCfg)
{
	init_mapping();

	init_screen_modules();

	init_sdcard();

	mode_rainbow_init();

	update_period_ms = RAINBOW_CYCLE_PERIOD;

	int32_t res = read_config(pCfg);

	if(res != 0)
	{
		while(res != 0)
		{
			//update 7seg display with error code
			switch(res)
			{
			case ((int32_t) (FR_NO_FILE)):
				ss_set_err_code(SS_ERR_CODE_NO_CFG_FILE);
				break;
			case ((int32_t) (FR_DISK_ERR)):
				ss_set_err_code(SS_ERR_CODE_NO_SD);
				break;
			}

			//wait some time
			wait_ms_blocking(250);

			//remount card
			FRESULT fr = f_mount(&fs, "4:", 1);
			if(fr != FR_OK)
			{
				while(fr != FR_OK)
				{
					ss_set_err_code(SS_ERR_CODE_NO_SD);

					//wait some time
					wait_ms_blocking(250);

					fr = f_mount(&fs, "4:", 1);
				}
			}

			//retry
			res = read_config(pCfg);
		}
	}
	return;
}

#define NUM_SONGS 4
int32_t read_config(song_conf * pCfg)
{
	FIL cfg_file;
	FRESULT fr;
	fr = f_open(&cfg_file, "config.txt", FA_READ);
	if(fr != FR_OK)
	{
		return ((int32_t)(fr));
	}

	//PARSE THE FILE AND INTIALIZE THE song effects
	char line[256];
	f_gets(line, 256, &cfg_file);
	uint32_t newSongIdx = 0;
	effect_params * pEndEffect;
	while( (fr == FR_OK) && (f_eof(&cfg_file) == 0))
	{
		//parse line
		char * pTok;
		pTok = strtok(line, ",");

		if((*pTok == ' ') || (*pTok == '\t'))
		{
			//another effect to the sequence
			//parse out time in seconds
			char * pTime = pTok;
			pTok = strtok(NULL, ",");
			//parse out effect
			effect_type effType = (effect_type) atoi(pTok);
			pEndEffect->pNextEff = malloc(sizeof(effect_params));
			fillEffect(effType, pEndEffect->pNextEff, line);
			pEndEffect = (effect_params *)(pEndEffect->pNextEff);
			uint32_t seconds = parseTime(pTime);
			pEndEffect->effectBeginSec = seconds;
			pEndEffect->pNextEff = NULL;
		}
		else
		{
			//new song
			pSongs[newSongIdx] = (song_effects*)malloc(sizeof(song_effects));
			switch(*pTok)
			{
			case('A'): //song effects will start after first beat detected
				pSongs[newSongIdx]->pFirstEff = ST_EFF_ACCEL;
				break;
			case('T'): //song effects will start after 3 seconds
			default:   //syntax error, assume 'T'
				pSongs[newSongIdx]->pFirstEff = ST_EFF_NORMAL;
				pTok = strtok(NULL, ",");
				effect_type effType = (effect_type) atoi(pTok);
				pSongs[newSongIdx]->pFirstEff = (effect_params *)malloc(sizeof(effect_params));
				fillEffect(effType, pSongs[newSongIdx]->pFirstEff, line);
				pSongs[newSongIdx]->pFirstEff->effectBeginSec = 0;
				pSongs[newSongIdx]->pFirstEff->pNextEff = NULL;
				break;
			}
			pEndEffect = pSongs[newSongIdx]->pFirstEff;
			newSongIdx++;
			if(newSongIdx >= MAX_NUM_SONGS)
				return 0;
		}

		//get next line
		f_gets(line, 256, &cfg_file);
	}
	pCfg->num_songs = newSongIdx;
	pCfg->effects_per_song = malloc(sizeof(uint32_t) * pCfg->num_songs);
	for(int a=0; a< pCfg->num_songs; a++)
	{
		effect_params * pEff = pSongs[a]->pFirstEff;
		uint32_t numEff = 0;
		while(pEff != NULL)
		{
			numEff++;
			pEff = pEff->pNextEff;
		}
		pCfg->effects_per_song[a] = numEff;
	}
	return 0;
}

uint32_t parseTime(char * pTime)
{
	uint32_t seconds;
	char * pTok = strtok(pTime, "\t:");
	seconds = 60 * (uint32_t)atoi(pTok);
	pTok = strtok(NULL, ":");
	seconds += (uint32_t)atoi(pTok);

	return seconds;
}
void fillEffect(effect_type effType, effect_params * pEff, char * pLine)
{
	if(pEff == NULL)
		return;

	char * pTok;
	pEff->effectType = effType;
	switch(effType)
	{
	case ET_PLAY_RAW_VIDEO:
		pTok = strtok(NULL, ',');
		pTok = strtok(pTok, "\"\r\n");
		char * pFilepath = malloc(sizeof(char) * (strlen(pTok) + 1) );
		if(pFilepath == NULL)
			return;
		strcpy(pFilepath, pTok);
		pEff->params.video.filepath = pFilepath;
		pEff->parseStatus = CFG_PARSE_OK;
		break;
	default:
		pEff->parseStatus = CFG_PARSE_ERR;
		return;
	}
}
void screen_show(void)
{
	while (update_in_progress);

	//wait for ws2813 reset based on last completed refresh
	while( get_ms_delta(update_time) < WS28XX_LATCH_MS );

	//start update process
	WS28XX_BIT_TIME_PERIPHERAL->CONTROLS[0].CnSC = 0x28;	//DISABLE DMA REQUESTS FROM FTM
	WS28XX_BIT_TIME_PERIPHERAL->CONTROLS[1].CnSC = 0x28;
	uint32_t cv = WS28XX_BIT_TIME_PERIPHERAL->CONTROLS[1].CnV;

	__disable_irq();

	// CAUTION: this code is timing critical.
	while (WS28XX_BIT_TIME_PERIPHERAL->CNT <= cv);
	while (WS28XX_BIT_TIME_PERIPHERAL->CNT > cv); // wait for beginning of an 800 kHz cycle
	while (WS28XX_BIT_TIME_PERIPHERAL->CNT < cv);
	WS28XX_BIT_TIME_PERIPHERAL->SC = 0;           // stop FTM2 timer (hopefully before it rolls over)
	WS28XX_BIT_TIME_PERIPHERAL->CNT = 0;
	update_in_progress = 1;

	PORTA->ISFR = 1<<10; // clear any prior rising edge
	uint32_t tmp __attribute__((unused));
	WS28XX_BIT_TIME_PERIPHERAL->CONTROLS[0].CnSC = 0x28;
	tmp = WS28XX_BIT_TIME_PERIPHERAL->CONTROLS[0].CnSC;         // clear any prior timer DMA triggers
	WS28XX_BIT_TIME_PERIPHERAL->CONTROLS[0].CnSC = 0x69;

	WS28XX_BIT_TIME_PERIPHERAL->CONTROLS[1].CnSC = 0x28;
	tmp = WS28XX_BIT_TIME_PERIPHERAL->CONTROLS[1].CnSC;
	WS28XX_BIT_TIME_PERIPHERAL->CONTROLS[1].CnSC = 0x69;

	DMA0->SERQ = 0; //ENABLE 3 channels
	DMA0->SERQ = 1;
	DMA0->SERQ = 2;
	WS28XX_BIT_TIME_PERIPHERAL->SC = FTM_SC_CLKS(1) | FTM_SC_PS(0); // restart FTM2 timer

	__enable_irq();
}

effect_params * pPrevEffect;
effect_params * pCurrEffect;
void updateScreen(void)
{
	switch(pCurrEffect->effectType)
	{
		case ET_PLAY_RAW_VIDEO:
			glediator_video();
			break;
	}
}

void reset_effect(void)
{
	switch(pCurrEffect->effectType)
	{
		case ET_PLAY_RAW_VIDEO:
			reset_glediator_video();
			break;
	}
}

void setPixel(uint32_t x, uint32_t y, rgb_led color)
{
	uint32_t ledIdx,stripIdx;
	if( map_mem[x][y].valid == 0)
		return;

	ledIdx = map_mem[x][y].led_index;
	stripIdx = map_mem[x][y].strip_index;

	if(ledIdx >= LEDS_PER_STRIP)
		return;
	if(stripIdx > 7)
		return;

	uint32_t * p = (uint32_t *)(&screen) + ledIdx * 6;
	uint32_t mask = 0x01010101 << stripIdx;
	uint32_t val=0;

	// Set bytes 0-3
	// G7 - G4 bits
	*p &= ~mask;
	val = ((((uint32_t)(color.g & 0x80)) >> 7) | (((uint32_t)(color.g & 0x40)) << 2) | (((uint32_t)(color.g & 0x20)) << 11) | (((uint32_t)(color.g & 0x10)) << 20)) << stripIdx;
	*p |= val;

	// Set bytes 4-7
	//g3 - g0
	*++p &= ~mask;
	val = ((((uint32_t)(color.g & 0x8)) >> 3) | (((uint32_t)(color.g & 0x4)) << 6) | (((uint32_t)(color.g & 0x2)) << 15) | (((uint32_t)(color.g & 0x1)) << 24)) << stripIdx;
	*p |= val;

	// Set bytes 8-11
	//R7 - R4
	*++p &= ~mask;
	val = ((((uint32_t)(color.r & 0x80)) >> 7) | (((uint32_t)(color.r & 0x40)) << 2) | (((uint32_t)(color.r & 0x20)) << 11) | (((uint32_t)(color.r & 0x10)) << 20)) << stripIdx;
	*p |= val;

	// Set bytes 12-15
	*++p &= ~mask;
	val = ((((uint32_t)(color.r & 0x8)) >> 3) | (((uint32_t)(color.r & 0x4)) << 6) | (((uint32_t)(color.r & 0x2)) << 15) | (((uint32_t)(color.r & 0x1)) << 24)) << stripIdx;
	*p |= val;

	// Set bytes 16-19
	*++p &= ~mask;
	val = ((((uint32_t)(color.b & 0x80)) >> 7) | (((uint32_t)(color.b & 0x40)) << 2) | (((uint32_t)(color.b & 0x20)) << 11) | (((uint32_t)(color.b & 0x10)) << 20)) << stripIdx;
	*p |= val;

	// Set bytes 20-23
	*++p &= ~mask;
	val = ((((uint32_t)(color.b & 0x8)) >> 3) | (((uint32_t)(color.b & 0x4)) << 6) | (((uint32_t)(color.b & 0x2)) << 15) | (((uint32_t)(color.b & 0x1)) << 24)) << stripIdx;
	*p |= val;
}

rgb_led getPixel(uint32_t ledIdx, uint32_t stripIdx)
{
	rgb_led ret = {0};
	if(ledIdx >= LEDS_PER_STRIP)
		return ret;
	if(stripIdx > 7)
		return ret;

	uint32_t * p = (uint32_t *)(&screen) + ledIdx * 6;
	uint32_t mask = 0x01010101 << stripIdx;
	ret.raw = 0;

	uint32_t four_bits = *p & mask;
	if(four_bits & 0xFF000000)
		ret.g |= 1 << 7;
	if(four_bits & 0xFF0000)
		ret.g |= 1 << 6;
	if(four_bits & 0xFF00)
		ret.g |= 1 << 5;
	if(four_bits & 0xFF)
		ret.g |= 1 << 4;
	four_bits = *++p & mask;
	if(four_bits & 0xFF000000)
		ret.g |= 1 << 3;
	if(four_bits & 0xFF0000)
		ret.g |= 1 << 2;
	if(four_bits & 0xFF00)
		ret.g |= 1 << 1;
	if(four_bits & 0xFF)
		ret.g |= 1;

	four_bits = *++p & mask;
	if(four_bits & 0xFF000000)
		ret.r |= 1 << 7;
	if(four_bits & 0xFF0000)
		ret.r |= 1 << 6;
	if(four_bits & 0xFF00)
		ret.r |= 1 << 5;
	if(four_bits & 0xFF)
		ret.r |= 1 << 4;
	four_bits = *++p & mask;
	if(four_bits & 0xFF000000)
		ret.r |= 1 << 3;
	if(four_bits & 0xFF0000)
		ret.r |= 1 << 2;
	if(four_bits & 0xFF00)
		ret.r |= 1 << 1;
	if(four_bits & 0xFF)
		ret.r |= 1;

	four_bits = *++p & mask;
	if(four_bits & 0xFF000000)
		ret.b |= 1 << 7;
	if(four_bits & 0xFF0000)
		ret.b |= 1 << 6;
	if(four_bits & 0xFF00)
		ret.b |= 1 << 5;
	if(four_bits & 0xFF)
		ret.b |= 1 << 4;
	four_bits = *++p & mask;
	if(four_bits & 0xFF000000)
		ret.b |= 1 << 3;
	if(four_bits & 0xFF0000)
		ret.b |= 1 << 2;
	if(four_bits & 0xFF00)
		ret.b |= 1 << 1;
	if(four_bits & 0xFF)
		ret.b |= 1;

	return ret;
}

int rainbowColors[180];
int colorIdx = 0;
void mode_rainbow_init(void)
{
	for(int i=0; i<180; i++)
	{
		int hue = i * 2;
		int sat = 100;
		int lightness = 25;
		rainbowColors[i] = makeColor(hue,sat,lightness);
	}
}
int makeColor(int h, int s, int l)
{
	unsigned int red, green, blue;
	unsigned int var1, var2;

	if (h > 359) h = h % 360;
	if (s > 100) s = 100;
	if (l > 100) l = 100;

	// algorithm from: http://www.easyrgb.com/index.php?X=MATH&H=19#text19
	if (s == 0) {
		red = green = blue = l * 255 / 100;
	} else {
		if (l < 50) {
			var2 = l * (100 + s);
		} else {
			var2 = ((l + s) * 100) - (s * l);
		}
		var1 = l * 200 - var2;
		red = h2rgb(var1, var2, (h < 240) ? h + 120 : h - 240) * 255 / 600000;
		green = h2rgb(var1, var2, h) * 255 / 600000;
		blue = h2rgb(var1, var2, (h >= 120) ? h - 120 : h + 240) * 255 / 600000;
	}
	return (red << 16) | (green << 8) | blue;
}
unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue)
{
	if (hue < 60) return v1 * 60 + (v2 - v1) * hue;
	if (hue < 180) return v2 * 60;
	if (hue < 240) return v1 * 60 + (v2 - v1) * (240 - hue);
	return v1 * 60;
}
void rainbowUpdate(void)
{
	//copy the color down the chain of leds

	for(int ledIdx = LEDS_PER_STRIP - 2; ledIdx >= 0; ledIdx--)
	{
		rgb_led tmp = getPixel(ledIdx,0);
		setPixel(ledIdx+1,0,tmp);
	}

	//update the first led
	rgb_led clr;
	clr.raw = rainbowColors[colorIdx++];
	colorIdx %= 180;
	setPixel(0, 0, clr);
}

void singleColorUpdate(void)
{
	static uint32_t y = 0;

	rgb_led clr;
	clr.raw = 0;
	clr.r = 0x00;
	clr.g = 0xff;
	clr.b = 0x00;

	rgb_led off;
	off.raw = 0;

	for(int row=0; row < MAP_ROWS; row++)
	{
		for(int col=0; col < MAP_COLUMNS; col++)
		{
			setPixel(col,row,off);
		}
	}

	for(int col=0; col < MAP_COLUMNS; col++)
	{
		setPixel(col,y,clr);
	}

	y++;
	if(y >= MAP_ROWS)
		y = 0;
}
const uint8_t gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

rgb_led gammaCorrect(rgb_led in)
{
	rgb_led out;
	out.r = gamma8[in.r];
	out.g = gamma8[in.g];
	out.b = gamma8[in.b];

	return out;

}

#define BELL_CURVE_LEN 181
const uint8_t bell_curve[BELL_CURVE_LEN] = {
	0,5,9,14,18,23,27,32,36,40,45,49,54,58,62,66,71,75,79,84,88,92,96,100,104,108,112,116,120,124,128,132,136,139,143,147,150,154,157,161,164,168,171,174,178,181,184,187,190,193,
	196,199,201,204,207,209,212,214,217,219,221,224,226,228,230,232,233,235,237,239,240,242,243,244,246,247,248,249,250,251,252,252,253,254,254,255,255,255,255,255,255,255,255,255,255,255,254,254,253,252,252,251,250,249,248,247,246,244,243,242,240,239,237,235,233,232,230,228,226,224,221,219,217,214,212,209,207,204,201,199,196,193,190,187,184,181,178,174,171,168,164,161,157,154,150,147,143,139,136,132,128,124,120,116,112,108,104,100,96,92,88,84,79,75,71,66,62,58,54,49,45,40,36,32,27,23,18,14,9,5,0
};

#define COLOR_REGION_LEN 16
#define AMERICA_OFF_DELAY 25
void americaFadeUpdate(void)
{
	rgb_led red,blue,white;
	static uint32_t bellIdx = 0;
	static uint32_t y = 0;
	if(bellIdx < BELL_CURVE_LEN)
	{
		red.raw = 0;
		blue.raw = 0;
		white.raw = 0;
		red.r = bell_curve[bellIdx];
		red.b = 0;
		red.g = 0;
		blue.b = bell_curve[bellIdx];
		blue.r = 0;
		blue.g = 0;
		white.r = bell_curve[bellIdx];
		white.g = bell_curve[bellIdx];
		white.b = bell_curve[bellIdx];

		red = gammaCorrect(red);
		blue = gammaCorrect(blue);
		white = gammaCorrect(white);
		//clear screen
		rgb_led off;
		off.raw = 0;
		for(int row=0; row < MAP_ROWS; row++)
		{
			for(int col=0; col < MAP_COLUMNS; col++)
			{
				setPixel(col,row,off);
			}
		}

		//set appropriate pixels to the colors above
		//red
		for(uint32_t pixelIdx = 0; pixelIdx < COLOR_REGION_LEN; pixelIdx++)
		{
			setPixel(pixelIdx,y,(red));
		}
		const uint32_t lastLedIdx = (MAP_COLUMNS - 1);
		//white
		for(uint32_t pixelIdx = COLOR_REGION_LEN; pixelIdx <= (lastLedIdx - COLOR_REGION_LEN); pixelIdx++)
		{
			setPixel(pixelIdx,y,(white));
		}
		//blue
		for(uint32_t pixelIdx = lastLedIdx; pixelIdx > (lastLedIdx - COLOR_REGION_LEN); pixelIdx--)
		{
			setPixel(pixelIdx,y,(blue));
		}
	}

	bellIdx++;
	if(bellIdx > (BELL_CURVE_LEN + AMERICA_OFF_DELAY))
		bellIdx = 0;
	y++;
	if(y > MAP_ROWS)
		y=0;
}

FIL vid_fil;
void reset_glediator_video(void)
{
	FRESULT res;
	if( (pCurrEffect == NULL) ||
		(pCurrEffect->effectType != ET_PLAY_RAW_VIDEO ) ||
		(pCurrEffect->parseStatus != CFG_PARSE_OK) ||
		(pCurrEffect->params.video.filepath == NULL))
		return;

	res = f_close(&vid_fil);
	res = f_open(&vid_fil, pCurrEffect->params.video.filepath, FA_READ);
	res = 2 + res;
}
void glediator_video(void)
{
	rgb frame[MAP_ROWS][MAP_COLUMNS];
	uint32_t bytes_read=0;
	f_read(&vid_fil, frame, (sizeof(rgb) * MAP_COLUMNS * MAP_ROWS), &bytes_read);
	if(bytes_read < (sizeof(rgb) * MAP_COLUMNS * MAP_ROWS))
	{
		reset_glediator_video();
		f_read(&vid_fil, frame, (sizeof(rgb) * MAP_COLUMNS * MAP_ROWS), &bytes_read);
	}

	for(int row=0; row < MAP_ROWS; row++)
	{
		for(int col=0; col < MAP_COLUMNS; col++)
		{
			rgb_led pixel;
			pixel.g = frame[row][col].g;
			pixel.r = frame[row][col].r;
			pixel.b = frame[row][col].b;
			setPixel(col,row,gammaCorrect(pixel));
			//col++;
		}
	}
}
