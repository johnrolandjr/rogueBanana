/*
 * time.h
 *
 *  Created on: Jan 28, 2019
 *      Author: Beau
 */

#ifndef TIME_H_
#define TIME_H_

#include "MK66F18.h"

extern uint32_t begin_loop_time;

typedef struct
{
	uint32_t sec;
	uint32_t ms;
}time_s_ms;

void LPTMR0_IRQHandler(void);

void time_init(void);
void reinit_time(uint32_t * pSec);
void update_main_loop_time(void);
uint32_t get_time_sec(void);
time_s_ms get_time(void);
uint32_t getTimeCount(void);
uint32_t get_ms_delta(time_s_ms begin);
void wait_ms_blocking(uint32_t ms);
void main_loop_period_wait(uint32_t period_ms);

#endif /* TIME_H_ */
