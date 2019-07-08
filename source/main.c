/*
 * Copyright (c) 2017, NXP Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
/**
 * @file    MK66FX1M0__PWM_GPIO.c
 * @brief   Application entry point.
 */
#include <button.h>
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "fsl_port.h"
#include "clock_config.h"
#include "MK66F18.h"
#include "ss_display_driver.h"
#include "time.h"
#include "screen.h"

/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */
#define SONG_SEL_TIME_MS 500

void mode_update(uint32_t changes);
void mode_wait(void);


/*
 * @brief   Application entry point.
 */
int main(void) {
  	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
  	/* Init FSL debug console. */
	BOARD_InitDebugConsole();

	display_init();
	song_conf conf;
	int32_t err = 0;
	do
	{
		err = screen_init(&conf);
	}while(err != 0);
	button_init(conf);

    //printf("Hello World\n");

	FTM_StartTimer(GEN_TIMER_10K_PERIPHERAL, kFTM_SystemClock);
    while(1) {
    	uint32_t modeChange = 0;
    	update_main_loop_time();
    	button_check(&modeChange);
    	mode_update(modeChange);
    	mode_wait();
    }
    return 0 ;
}

#define MOD_FLAG_BUTTON_EVENT	1
#define MOD_FLAG_TIME_EVENT		2
void mode_update(uint32_t changes)
{
	//update numerical display
	update_7seg_display();

	//update led display based on mode
	switch(get_mode())
	{
	case MODE_SONG_SEL:
	case MODE_SONG_START_COUNTDOWN:
		//no screen update
		break;
	case MODE_EFFECT_SEQ:
		updateScreen();
		screen_show();
		break;
	}
}

void mode_wait(void)
{
	//wait for the given amount of time depending on mode
	switch(get_mode())
	{
	case MODE_SONG_SEL:
		main_loop_period_wait((SONG_SEL_TIME_MS/SONG_SEL_TIME_CHECKS));
		break;
	case MODE_SONG_START_COUNTDOWN:
		wait_ms_blocking(1000);
		break;
	case MODE_EFFECT_SEQ:
		main_loop_period_wait(update_period_ms);
		break;
	}
}
