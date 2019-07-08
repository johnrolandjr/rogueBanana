#include "time.h"
#include "fsl_lptmr.h"

static uint32_t main_begin_time_ms = 0;
static time_s_ms sys_time = {0,0};
uint32_t begin_loop_time = 0;

/*
 * This occurs when the low power timer compare is met. Waited the expected time.
 * Update system time to reflect changes.
 */
void LPTMR0_IRQHandler(void)
{
	sys_time.ms += 2;
	if(sys_time.ms > 999)
	{
		sys_time.ms -= 1000;
		sys_time.sec++;
	}

	//ACK the interrupt
	uint32_t reg = LPTMR0->CSR;
	reg |= LPTMR_CSR_TCF_MASK;
	LPTMR0->CSR = reg;
}
void time_init(void)
{
	// First, Configure LPTMR - Generic Low Power Timer using RTC, since we got it,
	// as a way to know the time, know when we need to wait before latching the colors in the leds
	lptmr_config_t gen_timer_cfg;
	LPTMR_GetDefaultConfig(&gen_timer_cfg);
	gen_timer_cfg.enableFreeRunning = false;
	gen_timer_cfg.prescalerClockSource = kLPTMR_PrescalerClock_1; // LPO - 1kHz internal clk source
	gen_timer_cfg.bypassPrescaler = true;
	LPTMR_Init(LPTMR0, &gen_timer_cfg);

	LPTMR0->CMR = 1;	//1 ticks = 1ms
	LPTMR_EnableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);

	//enable the lptmr clock
	LPTMR0->CSR |= 1;

	EnableIRQ(LPTMR0_IRQn);
}

void reinit_time(uint32_t * pSec)
{
	if(pSec == NULL)
	{
		sys_time.sec = 0;
	}
	else
	{
		sys_time.sec = *pSec;
	}
	sys_time.ms = 0;
}
void update_main_loop_time(void)
{
	main_begin_time_ms = sys_time.ms;
}
uint32_t get_time_sec(void)
{
	return sys_time.sec;
}
time_s_ms get_time(void)
{
	return sys_time;
}
uint32_t getTimeCount(void)
{
    /* Must first write any value to the CNR. This synchronizes and registers the current value
     * of the CNR into a temporary register which can then be read
     */
	LPTMR0->CNR = 0U;
    return (uint32_t)(LPTMR0->CNR & LPTMR_CNR_COUNTER_MASK);
}

uint32_t get_ms_delta(time_s_ms begin)
{
	uint32_t delta_ms;
	time_s_ms end = get_time();
	if(begin.sec == end.sec)
	{
		delta_ms = (end.ms - begin.ms);
	}
	else
	{
		delta_ms = (1000 - begin.ms + end.ms);
	}
	return delta_ms;
}

void wait_ms_blocking(uint32_t ms)
{
	time_s_ms current_time = get_time();
	time_s_ms return_time = current_time;

	return_time.ms += ms;
	if(return_time.ms >= 999)
	{
		return_time.sec += (return_time.ms / 1000);
		return_time.ms %= 1000;
	}

	do
	{
		current_time = get_time();
	}while( (current_time.sec < return_time.sec) || (current_time.ms < return_time.ms));
}
/*
 * Function that waits the amount of time necessary to begin the main loop at the desired period
 */
void main_loop_period_wait(uint32_t period_ms)
{
	uint32_t next_start_time_ms = main_begin_time_ms + period_ms;
	static uint32_t prev_sec = 0;
	if(next_start_time_ms >= 999)
	{
		//time will have to wrap
		while((sys_time.ms < next_start_time_ms) && (sys_time.sec == prev_sec)){};
	}
	else
	{
		//wait remaining time
		while(sys_time.ms < next_start_time_ms){};
	}
	prev_sec = sys_time.sec;
}
