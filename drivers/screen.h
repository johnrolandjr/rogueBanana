/*
 * screen.h
 *
 *  Created on: Dec 3, 2018
 *      Author: Beau
 */

#ifndef SCREEN_H_
#define SCREEN_H_

#include "fsl_edma.h"
#include "fsl_lptmr.h"
#include "fsl_sdhc.h"
#include "ff.h"
#include "peripherals.h"

//STRUCTS
typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
}rgb;
typedef union
{
	struct{
		uint8_t r;
		uint8_t b;
		uint8_t g;
		uint8_t dummy;
	};
	uint32_t raw;
}rgb_led;

typedef struct
{
	uint32_t valid:1;
	uint32_t strip_index:3;
	uint32_t led_index:28;
}x_y;

typedef struct
{
	uint32_t strip_index;
	uint32_t led_index;
	struct r_theta * p_next;
}r_theta;

typedef union
{
	r_theta ** rtheta;
}screenMapping;

typedef struct
{
	uint32_t num_songs;
	uint32_t * effects_per_song;
}song_conf;

typedef enum
{
	CFG_PARSE_OK = 0,
	CFG_PARSE_ERR
}eff_parse_status;
typedef enum
{
	ET_PLAY_RAW_VIDEO = 1
}effect_type;
typedef enum
{
	ST_EFF_NORMAL = 0,
	ST_EFF_ACCEL
}start_effect_type;
typedef struct
{
	char * filepath;
}raw_video_param;
typedef struct
{
	eff_parse_status parseStatus;
	uint32_t effectBeginSec;
	effect_type effectType;
	union
	{
		raw_video_param video;
	}params;
	struct effect_params * pNextEff;
}effect_params;
typedef struct
{
	start_effect_type startType;
	uint32_t endTimeSec;
	effect_params * pFirstEff;
}song_effects;

extern uint32_t update_period_ms;
extern effect_params * pPrevEffect;
extern effect_params * pCurrEffect;
#define MAX_NUM_SONGS 20
extern song_effects * pSongs[MAX_NUM_SONGS];

//MACROS
#define LEDS_PER_STRIP  200
#define BITS_PER_LED	24

#define WS28XX_LATCH_MS 1

#define DEFAULT_LAST_EFF_DURATION_SEC 30

#define RAINBOW_CYCLE_PERIOD 33
//public prototype declarations
int32_t screen_init(song_conf * pCfg);
int32_t read_config(song_conf * pCfg);
void screen_show(void);
void init_sdcard(void);
void updateScreen(void);
void fillEffect(effect_type effType, effect_params * pEff, char * pLine);
uint32_t parseTime(char * pTime);
void screen_update_reset(void);

//mode prototypes
void mode_rainbow_init(void);
void rainbowUpdate(void);
void americaFadeUpdate(void);
void glediator_video(void);
void clearScreen(void);
void singleColorUpdate(void);

void reset_effect(void);
void reset_glediator_video(void);
void close_glediator_video(void);

//helper functions
unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue);
int makeColor(int h, int s, int l);
rgb_led gammaCorrect(rgb_led in);
#endif /* SCREEN_H_ */
