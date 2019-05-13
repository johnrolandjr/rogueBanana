/*
 * time.h
 *
 *  Created on: Jan 28, 2019
 *      Author: Beau
 */

#ifndef TIME_H_
#define TIME_H_

#include "MK66F18.h"

void time_init(void);
uint32_t getTimeCount(void);
uint32_t get_ms_delta(uint32_t begin);
void wait_ms_blocking(uint32_t ms);

#endif /* TIME_H_ */
