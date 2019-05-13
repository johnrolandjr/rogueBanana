#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include "button.h"

//MACRO
#define SS_CLK_PIN 0	//PTB0
#define SS_RST_PIN 1	//PTB1
#define SS_OUT_PIN 3	//PTB3

#define MAX_DIS_NUM 99
#define DIS_IDX_L 0
#define DIS_IDX_R 1
#define NUM_DIS_MODS 2

#define NUM_DISPLAYS 4
#define NUM_BITS_PER_DISPLAY 8

#define SONG_SEL_TIME_CHECKS 5

//typedef
typedef enum
{
	SS_ERR_CODE_NO_SD = 1,
	SS_ERR_CODE_NO_CFG_FILE
}SS_ERR_CODES;
// prototype declarations
void display_init(void);
void update_7seg_display(void);
void display_set_num(int dis_sel, int num);
void display_set_sym(int dis_sel, char * pSymbal);
void display_set_raw(int dis_sel, int raw);
void ss_set_err_code(SS_ERR_CODES error);

unsigned int get_mode(void);
void set_mode(mode mod);
unsigned int get_opt(void);

void inc_mode(void);
void inc_opt(void);

void delay_ms(unsigned int ms);
void delay_us(unsigned int us);
void delay_ns(unsigned int ns);
void wait_cycles( unsigned int n);

#endif
