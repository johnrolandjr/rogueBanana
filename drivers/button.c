#include <button.h>
#include "time.h"
#include "screen.h"
#include "MK66F18.h"

GPIO_Type * pButtons = GPIOB_BASE;

static uint32_t button_states[NUM_BUTTON_STATES] = {0};
static uint32_t button_1_pressed_time = 0;
static uint32_t button_2_pressed_time = 0;
button_state button_1_state = BUTTON_STATE_UNPRESSED;
button_state button_2_state = BUTTON_STATE_UNPRESSED;

FTM_Type * ftm_debounce = ((FTM_Type *)FTM0_BASE);
#define DUR_TICK_INIT_CNT 0
#define DUR_TICK_SHORT_PRESS_THRESH 1000
#define DUR_TICK_LONG_HOLD_THRESH 10000
unsigned int duration_tick_held[2] = {0,0};
uint32_t button_held_ack = 0;
void GEN_TIMER_10K_IRQHANDLER(void)
{
	static uint32_t idx=0;
	static int prev_debounce = 0x0;

	button_states[idx] = ~(pButtons->PDIR);
	idx++;

	int debounce = 0xFFFFFFFF;
	for(int a=0; a < NUM_BUTTON_STATES; a++)
	{
		debounce &= button_states[a];
	}

	int change = debounce ^ prev_debounce;

	if(debounce & BUTTON_MASK)
	{
		//increment counter if high
		if(debounce & (1 << BUTTON_1_PIN))
		{
			duration_tick_held[0]++;
			if(duration_tick_held[0] > DUR_TICK_LONG_HOLD_THRESH)
			{
				button_1_state = BUTTON_STATE_HELD;
			}
		}
		if(debounce & (1 << BUTTON_2_PIN))
		{
			duration_tick_held[1]++;
			if(duration_tick_held[1] > DUR_TICK_LONG_HOLD_THRESH)
			{
				button_2_state = BUTTON_STATE_HELD;
			}
		}
	}

	if(change & BUTTON_MASK)
	{
		if(change & (1 << BUTTON_1_PIN))
		{
			if(debounce & (1 << BUTTON_1_PIN))
			{
				//Rised - Reinit
				duration_tick_held[0] = 0;
				if(button_held_ack == 1)
				{
					button_1_state = BUTTON_STATE_UNPRESSED;
					button_held_ack = 0;
				}
			}
			else
			{
				//Lowered - Set state
				if(	(duration_tick_held[0] > DUR_TICK_SHORT_PRESS_THRESH) &&
					(duration_tick_held[0] <= DUR_TICK_LONG_HOLD_THRESH))
				{
					button_1_state = BUTTON_STATE_PRESSED;
				}
			}
		}
		if(change & (1 << BUTTON_2_PIN))
		{
			if(debounce & (1 << BUTTON_2_PIN))
			{
				//Rised - Reinit
				duration_tick_held[1] = 0;
				if(button_held_ack == 1)
				{
					button_2_state = BUTTON_STATE_UNPRESSED;
					button_held_ack = 0;
				}
			}
			else
			{
				//Lowered - Set state
				if((duration_tick_held[1] > DUR_TICK_SHORT_PRESS_THRESH) &&
					(duration_tick_held[1] <= DUR_TICK_LONG_HOLD_THRESH))
				{
					button_2_state = BUTTON_STATE_PRESSED;
				}
			}
		}
	}

	/*
	if(change & BUTTON_MASK)
	{

		if(change & (1 << BUTTON_1_PIN))
		{
			//button 1
			if(debounce & (1 << BUTTON_1_PIN))
			{
				//released
				int time_dif = get_ms_delta(button_1_pressed_time);
				if(time_dif > BUTTON_PRESS_MIN_DUR_MS)
				{
					button_1_state = BUTTON_STATE_HELD;
				}
				else
				{
					button_1_state = BUTTON_STATE_PRESSED;
				}
			}
			else
			{
				//pressed
				button_1_pressed_time = getTimeCount();
			}
		}

		if(change & (1 << BUTTON_2_PIN))
		{
			//button 2
			if(debounce & (1 << BUTTON_2_PIN))
			{
				//released
				int time_dif = get_ms_delta(button_2_pressed_time);
				if(time_dif > BUTTON_PRESS_MIN_DUR_MS)
				{
					button_2_state = BUTTON_STATE_HELD;
				}
				else
				{
					button_2_state = BUTTON_STATE_PRESSED;
				}
			}
			else
			{
				//pressed
				button_2_pressed_time = getTimeCount();
			}
		}
	}
	*/

	if(idx >= NUM_BUTTON_STATES)
		idx = 0;
	prev_debounce = debounce;

	//acknowledge interrupt
	ftm_debounce->STATUS &= ~(1 & 0xFFU);
}

static mode curr_mode = MODE_SONG_SEL;
static uint32_t song_idx = 0;
static uint32_t num_songs = 1;
static uint32_t * song_num_effects;
static uint32_t effect_idx = 0;
uint32_t flag_button_changed_operation = 0;

void button_init(song_conf cfg)
{
	num_songs = cfg.num_songs;
	song_num_effects = cfg.effects_per_song;
}

uint32_t get_song_idx(void)
{
	return song_idx;
}
uint32_t get_effect_idx(void)
{
	return effect_idx;
}

void button_check(uint32_t* buttonState)
{
	/*
	// implement xor of button states
	// if only 1 button is pressed
	if( !(button_1_state != BUTTON_STATE_UNPRESSED) !=
		!(button_2_state != BUTTON_STATE_UNPRESSED))
	{
		flag_button_changed_operation = 1;
		if( (button_1_state == BUTTON_STATE_HELD) ||
			(button_2_state == BUTTON_STATE_HELD))
		{
			button_held();
		}
		else
		{
			if(button_1_state != BUTTON_STATE_UNPRESSED)
				button_1_pressed();
			else
				button_2_pressed();
		}
	}
	else if((button_1_state == BUTTON_STATE_HELD) &&
			(button_2_state == BUTTON_STATE_HELD))
	{
		flag_button_changed_operation = 1;
		button_held();
	}

	*buttonState = flag_button_changed_operation;
	*/
	static prev_button_states[2] = {BUTTON_STATE_UNPRESSED,BUTTON_STATE_UNPRESSED};
	if(button_1_state == BUTTON_STATE_PRESSED)
	{
		button_1_pressed();
	}
	if(button_2_state == BUTTON_STATE_PRESSED)
	{
		button_2_pressed();
	}
	if(	(button_1_state == BUTTON_STATE_HELD && prev_button_states[0] != BUTTON_STATE_HELD) ||
		(button_2_state == BUTTON_STATE_HELD && prev_button_states[1] != BUTTON_STATE_HELD))
	{
		button_held();
		button_held_ack = 1;
	}
	prev_button_states[0] = button_1_state;
	prev_button_states[1] = button_2_state;
}

mode get_mode(void)
{
	return curr_mode;
}
void set_mode(mode mod)
{
	curr_mode = mod;
}

void button_1_pressed(void)
{
	if(button_1_state == BUTTON_STATE_PRESSED)
	{
		switch(curr_mode)
		{
		case MODE_SONG_SEL:
			//button_1 pressed = previous song
			if(song_idx == 0)
				song_idx = num_songs - 1;
			else
				song_idx--;
			break;
		case MODE_EFFECT_SEQ:
			//button 1 pressed = previous effect
			effect_idx++;
			if(effect_idx >= song_num_effects[song_idx])
			{
				effect_idx = 0;
			}
			break;
		case MODE_SONG_START_COUNTDOWN:
			//cancel selection
			set_mode(MODE_SONG_SEL);
			break;
		}
	}
	button_1_state = BUTTON_STATE_UNPRESSED;
}

void button_2_pressed(void)
{
	if(button_2_state == BUTTON_STATE_PRESSED)
	{
		switch(curr_mode)
		{
		case MODE_SONG_SEL:
			//button_2 pressed = next song
			song_idx++;
			if(song_idx >= num_songs)
				song_idx = 0;
			break;
		case MODE_EFFECT_SEQ:
			//button 2 pressed = next effect
			effect_idx++;
			if(effect_idx >= song_num_effects[song_idx])
			{
				effect_idx = 0;
			}
			break;
		case MODE_SONG_START_COUNTDOWN:
			//cancel selection
			set_mode(MODE_SONG_SEL);
			break;
		}
	}
	button_2_state = BUTTON_STATE_UNPRESSED;
}

int32_t countdown=INIT_COUNTDOWN_SEC;
void button_held(void)
{
	if( curr_mode == MODE_SONG_START_COUNTDOWN)
	{
		set_mode(MODE_SONG_SEL);
	}
	else
	{
		curr_mode++;
		if(curr_mode >= MODE_EFFECT_SEQ)
			curr_mode = MODE_SONG_SEL;

		//reset mode
		switch(curr_mode)
		{
		case MODE_SONG_SEL:
		case MODE_EFFECT_SEQ:
			//nothing
			break;
		case MODE_SONG_START_COUNTDOWN:
			pPrevEffect = pSongs[get_song_idx()]->pFirstEff;
			pCurrEffect = pPrevEffect;
			reset_effect();
			countdown = INIT_COUNTDOWN_SEC;
			break;
		}

	}
}


