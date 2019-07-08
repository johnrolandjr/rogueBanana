/*
 * button.h
 *
 *  Created on: Jan 28, 2019
 *      Author: Beau
 */

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "MK66F18.h"
#include "screen.h"

#define BUTTON_1_PIN 10
#define BUTTON_2_PIN 11
#define BUTTON_MASK ((1 << BUTTON_1_PIN) | (1 << BUTTON_2_PIN))

#define NUM_BUTTON_STATES 25
#define BUTTON_PRESS_MIN_DUR_MS 800

#define INIT_COUNTDOWN_SEC 3
extern int32_t countdown;

typedef enum
{
	BUTTON_STATE_UNPRESSED = 0,
	BUTTON_STATE_PRESSED,
	BUTTON_STATE_HELD
}button_state;

typedef enum
{
	MODE_SONG_SEL = 0,
	MODE_SONG_START_COUNTDOWN,
	MODE_EFFECT_SEQ
}mode;

//to be shared with screen to signal that the effect has changed, therefore,
//change/reset outputs accordingly
extern uint32_t flag_button_changed_operation;
extern button_state button_1_state;
extern button_state button_2_state;

void button_init(song_conf cfg);
uint32_t get_song_idx(void);
uint32_t get_effect_idx(void);
void inc_effect_idx(void);
void button_check(uint32_t * buttonState);
void button_1_pressed(void);
void button_2_pressed(void);
void button_held(void);

void debounce(void);

#endif /* BUTTON_H_ */
