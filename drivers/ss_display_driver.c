#include <button.h>
#include "ss_display_driver.h"
#include "MK66F18.h"

#define CYCLES_PER_LOOP 3
inline void wait_cycles( unsigned int n ) {
	unsigned int l = n/CYCLES_PER_LOOP;
    asm volatile( "0:" "SUBS %[count], 1;" "BNE 0b;" :[count]"+r"(l) );
}

//internal function prototypes
void clock_bit(void);
void display_reset(void);
unsigned int get_serial_encode(int number);

static unsigned int raw_val=0;
unsigned int left_num=0;
unsigned int right_num=0;

unsigned int ss_numbers[10]={0x21,0xF9,0x13,0x51,0xC9,0x45,0x05,0xF1,0x01,0xC1};
#define NUM_SYM 8
unsigned int ss_symbols[NUM_SYM]={0xDF, 0xFF, 0x45, 0x19, 0x07, 0x80, 0x80, 0x87};
char ss_sym_lookup[NUM_SYM] = { '-', ' ', 'S', 'd', 'E', 'R', 'A', 'F'};

//global functions
void display_init(void)
{
	display_reset();

    /* Test Code */
    //cycle through numbers
    for(int a=0; a<=9; a++)
    {
        display_set_num(DIS_IDX_L, a*11);
        display_set_num(DIS_IDX_R, a*11);
        delay_ms(100);
    }

    update_7seg_display();
}

void update_7seg_display(void)
{
	static int counter=0;
    mode mod = get_mode();
    if(mod == MODE_SONG_SEL)
    {
    	uint32_t songIdx = get_song_idx();
    	if(counter == 0)
		{
    		display_set_sym(DIS_IDX_L, "--");
    		display_set_num(DIS_IDX_R, songIdx+1);
		}else if (counter == SONG_SEL_TIME_CHECKS)
		{
			display_set_sym(DIS_IDX_L, "  ");
			display_set_num(DIS_IDX_R, songIdx+1);
		}
    	counter++;
    	if(counter >= (SONG_SEL_TIME_CHECKS << 1))
    		counter = 0;
    }
    else if(mod == MODE_SONG_START_COUNTDOWN)
    {
    	uint32_t songIdx = get_song_idx();
    	display_set_num(DIS_IDX_L, songIdx+1);
    	display_set_num(DIS_IDX_R, countdown);
    	countdown--;
    	if(countdown < 0)
    	{
    		//move to next mode
    		//for now move directly into effect sequence
    		set_mode(MODE_EFFECT_SEQ);
    	}
    }
    else
    {
    	//mod == MODE_EFFECT_SEQ
    	uint32_t songIdx = get_song_idx();
    	uint32_t effIdx = get_effect_idx();
    	display_set_num(DIS_IDX_L, songIdx+1);
    	display_set_num(DIS_IDX_R, effIdx);
    }
}
void display_set_sym(int dis_sel, char * pSymbal)
{
	//error check inputs
	if((dis_sel >= NUM_DIS_MODS) || (pSymbal == NULL))
		return;

	int raw = 0;
	int bFound = 1;
	for(int b=0; b < 2; b++)
	{
		if(bFound == 1)
		{
			for(int a = 0; a < NUM_SYM; a++)
			{
				char * pChar = pSymbal+b;
				char * pSym = &ss_sym_lookup[a];
				if( *pChar == *pSym )
				{
					raw <<= 8;
					raw |= (ss_symbols[a] & 0xff);
					break;
				}
				if(a == NUM_SYM - 1)
					bFound = 0;
			}
		}
	}

	if(bFound == 1)
	{
		display_set_raw(dis_sel, raw);
	}
}
void display_set_raw(int dis_sel, int raw)
{
    //UPDATE raw_val
    if(dis_sel == DIS_IDX_R)
    {
        raw_val &= 0xFFFF0000;
        raw_val |= raw & 0xFFFF;
    }
    else if(dis_sel == DIS_IDX_L)
    {
        raw_val &= 0x0000FFFF;
        raw_val |= (raw & 0xFFFF) << 16;
    }
    
    //shift raw_val to the 7 seg displays
    //assert reset line
    unsigned int bit = 1;
    for(int a=0; a<32; a++)
    {
        if(bit & raw_val)
        	GPIOB->PSOR = 1 << SS_OUT_PIN;
        else
        	GPIOB->PCOR = 1 << SS_OUT_PIN;
        //set serial shift out to out val
        
        clock_bit();
        
        bit <<= 1;
    }
}

void ss_set_err_code(SS_ERR_CODES error){
	switch(error)
	{
	case SS_ERR_CODE_NO_SD:
		display_set_sym(DIS_IDX_L, "E-");
		display_set_sym(DIS_IDX_R, "Sd");
		break;
	case SS_ERR_CODE_NO_CFG_FILE:
		display_set_sym(DIS_IDX_L, "E-");
		display_set_sym(DIS_IDX_R, " F");
		break;
	}
}
void display_set_num(int dis_sel, int num)
{
    unsigned int ss_num=0;

    //error check inputs
    if((num > MAX_DIS_NUM) || (dis_sel >= NUM_DIS_MODS ))
        return;

    ss_num = get_serial_encode(num);

    display_set_raw(dis_sel, ss_num);
}
        
void delay_ms(unsigned int ms)
{
	for(int a=0; a<ms; a++)
	{
		delay_us(1000);
	}
}
void delay_us(unsigned int us)
{
	//if(us > 5)
	//{
		us *= 11111;
		us >>= 10;
	//}
	while(us != 0)
	{
		__NOP();
		us--;
	}
}

//internal functions
void clock_bit(void)
{
	GPIOB->PSOR = 1 << SS_CLK_PIN;
    delay_us(1);
    GPIOB->PCOR = 1 << SS_CLK_PIN;
    delay_us(1);
}
void display_reset(void)
{
	GPIOB->PCOR = 1 << SS_RST_PIN;
	delay_us(1);
	GPIOB->PSOR = 1 << SS_RST_PIN;
}
unsigned int get_serial_encode(int number)
{
    unsigned int tens=0;
    unsigned int ones=0;
    unsigned int res=0;
    
    ones = number % 10;
    tens = (number / 10) % 10;
    
    res |= ss_numbers[ones];
    res |= ss_numbers[tens] << 8;
    
    return res;
}
