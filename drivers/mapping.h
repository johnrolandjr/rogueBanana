
#ifndef MAP_H
#define MAP_H_

typedef struct
{
	uint32_t strip_begin_led_idx;
	uint32_t strip_idx;
	uint32_t begin_x;
	uint32_t end_x;
	uint32_t begin_y;
	uint32_t end_y;
}rect_mapping;

#define MAP_COLUMNS 25 //49
#define MAP_ROWS	72

#define MAP_RECT_STRIPS 21
#define MAP_RECT_MAX_LEDS_PER_STRIP 188
const rect_mapping xy_strip_mapping[MAP_RECT_STRIPS] = \
{
	/*//original mappy with spaces
	{0,6,0,0,24,49},
	{26,6,4,4,56,15},
	{68,6,8,8,9,62},
	{0,5,10,10,14,58},
	{45,5,12,12,66,4},
	{0,1,14,14,14,58},
	{45,1,16,16,68,3},
	{111,1,18,18,13,58},
	{0,3,20,20,2,70},
	{69,3,22,22,58,13},
	{0,0,28,28,1,70},
	{70,0,26,26,58,13},
	{116,0,24,24,0,71},
	{0,7,32,32,3,68},
	{66,7,30,30,58,13},
	{0,4,38,38,13,58},
	{46,4,36,36,67,5},
	{109,4,34,34,13,58},
	{69,2,40,40,9,62},
	{27,2,44,44,56,15},
	{0,2,48,48,23,49},
	*/
	//new mapping
	{0,6,0,0,24,49},
	{26,6,2,2,56,15},
	{68,6,4,4,9,62},
	{0,5,5,5,14,58},
	{45,5,6,6,66,4},
	{0,1,7,7,14,58},
	{45,1,8,8,68,3},
	{111,1,9,9,13,58},
	{0,3,10,10,2,70},
	{69,3,11,11,58,13},
	{0,0,14,14,1,70},
	{70,0,13,13,58,13},
	{116,0,12,12,0,71},
	{0,7,16,16,3,68},
	{66,7,15,15,58,13},
	{0,4,19,19,13,58},
	{46,4,18,18,67,5},
	{109,4,17,17,13,58},
	{69,2,20,20,9,62},
	{27,2,22,22,56,15},
	{0,2,24,24,23,49},
};

#endif //MAP_H_
