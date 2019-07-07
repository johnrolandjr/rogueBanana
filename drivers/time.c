#include "time.h"
#include "fsl_lptmr.h"

static uint32_t seconds = 0;

void time_init(void)
{
	// First, Configure LPTMR - Generic Low Power Timer using RTC, since we got it,
	// as a way to know the time, know when we need to wait before latching the colors in the leds
	lptmr_config_t gen_timer_cfg;
	LPTMR_GetDefaultConfig(&gen_timer_cfg);
	gen_timer_cfg.enableFreeRunning = true;
	gen_timer_cfg.value = kLPTMR_Prescale_Glitch_12; // GLITCH 10 divide by 2^x -> 2048
													 // 2Mhz -> 976.56Hz = 1.024ms / tick
													 // 16bit period means .. maximum period 63.999 seconds = 1 min 4 sec
	gen_timer_cfg.bypassPrescaler = false;
	gen_timer_cfg.prescalerClockSource = kLPTMR_PrescalerClock_0; //MCGIRCLK - 2MHz internal clk source
	LPTMR_Init(LPTMR0, &gen_timer_cfg);

	LPTMR_EnableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);

	//enable the lptmr clock
	LPTMR0->CSR |= 1;
}

uint32_t getTimeCount(void)
{
    /* Must first write any value to the CNR. This synchronizes and registers the current value
     * of the CNR into a temporary register which can then be read
     */
	LPTMR0->CNR = 0U;
    return (uint32_t)(LPTMR0->CNR & LPTMR_CNR_COUNTER_MASK);
}

uint32_t get_ms_delta(uint32_t begin)
{
	uint32_t end, delta_ms;
	end = LPTMR_GetCurrentTimerCount(LPTMR0);
	if(end > begin)
	{
		delta_ms = (end - begin);
	}
	else
	{
		delta_ms = (0x10000 - begin + end);
	}
	delta_ms <<= 2;
	return delta_ms;
}

void wait_ms_blocking(uint32_t ms)
{
	uint32_t ticks = (uint32_t)((float) ms / 4.096f);
	if(ticks == 0)
		return;	//TODO, perform some other blocking wait for less than 4 ms

	uint32_t ret_time = (getTimeCount() + ticks);
	if(ret_time > 0xffff)
	{
		ret_time -= 0xffff;
		while(getTimeCount() != 0xFFFF);
	}

	while(getTimeCount() < ret_time);
}
