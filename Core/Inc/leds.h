/*
 * leds.h
 *
 *  Created:	25-Sept-2026
 *  Author:		David Waskevich
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LEDS_H
#define __LEDS_H

#include <stdint.h>

/* enum to identify and define individual LEDs (from top to bottom) ... 53 in total */
enum 
{
	arc_top_1,
	arc_top_2,
	arc_top_3,
	arc_top_4,
	arc_top_5,
	lightning_bolt,
	one_top_upper,
	one_top_lower,
	digit_top_left_a,  digit1_top_a = digit_top_left_a,
	digit_top_left_b,  digit1_top_b = digit_top_left_b,
	digit_top_left_c,  digit1_top_c = digit_top_left_c,
	digit_top_left_d,  digit1_top_d = digit_top_left_d,
	digit_top_left_e,  digit1_top_e = digit_top_left_e,
	digit_top_left_f,  digit1_top_f = digit_top_left_f,
	digit_top_left_g,  digit1_top_g = digit_top_left_g,
	digit_top_right_a, digit0_top_a = digit_top_right_a,
	digit_top_right_b, digit0_top_b = digit_top_right_b,
	digit_top_right_c, digit0_top_c = digit_top_right_c,
	digit_top_right_d, digit0_top_d = digit_top_right_d,
	digit_top_right_e, digit0_top_e = digit_top_right_e,
	digit_top_right_f, digit0_top_f = digit_top_right_f,
	digit_top_right_g, digit0_top_g = digit_top_right_g,
	bar_top_left,
	bar_top_right,
	cycle_nw,
	cycle_ne,
	rocket,
	cycle_sw,
	cycle_se,
	bar_bot_left,
	bar_bot_right,
	one_bot_upper,
	one_bot_lower,
	digit_bot_left_a,  digit1_bot_a = digit_bot_left_a,
	digit_bot_left_b,  digit1_bot_b = digit_bot_left_b,
	digit_bot_left_c,  digit1_bot_c = digit_bot_left_c,
	digit_bot_left_d,  digit1_bot_d = digit_bot_left_d,
	digit_bot_left_e,  digit1_bot_e = digit_bot_left_e,
	digit_bot_left_f,  digit1_bot_f = digit_bot_left_f,
	digit_bot_left_g,  digit1_bot_g = digit_bot_left_g,
	digit_bot_right_a, digit0_bot_a = digit_bot_right_a,
	digit_bot_right_b, digit0_bot_b = digit_bot_right_b,
	digit_bot_right_c, digit0_bot_c = digit_bot_right_c,
	digit_bot_right_d, digit0_bot_d = digit_bot_right_d,
	digit_bot_right_e, digit0_bot_e = digit_bot_right_e,
	digit_bot_right_f, digit0_bot_f = digit_bot_right_f,
	digit_bot_right_g, digit0_bot_g = digit_bot_right_g,
	tear_drop,
	arc_bot_1,
	arc_bot_2,
	arc_bot_3,
	arc_bot_4,
	arc_bot_5,
	MAX_LEDS
};

typedef struct
{
	uint8_t  scan_line;
	uint32_t set_segment_GPIOA;
	uint32_t set_segment_GPIOB;
	uint32_t reset_segment_GPIOA;
	uint32_t reset_segment_GPIOB;
} LED_port_position_t;

const static LED_port_position_t LED_port_position[MAX_LEDS] =
{
/*{scan, setPortA, setPortB,  rstPortA, rstPortB} */
	{0, 0x00000000, 0x00000100, 0x00000000, 0x01000000}, 	// 0  - arc_top_1,
	{0, 0x00000000, 0x00000080, 0x00000000, 0x00800000}, 	// 1  - arc_top_2,
	{0, 0x00000000, 0x00000040, 0x00000000, 0x00400000}, 	// 2  - arc_top_3,
	{0, 0x00000000, 0x00000020, 0x00000000, 0x00200000}, 	// 3  - arc_top_4,
	{0, 0x00000000, 0x00000008, 0x00000000, 0x00080000}, 	// 4  - arc_top_5,
	{0, 0x00008000, 0x00000000, 0x80000000, 0x00000000}, 	// 5  - lightning_bolt,
	{6, 0x00000800, 0x00000000, 0x08000000, 0x00000000}, 	// 6  - one_top_upper,
	{5, 0x00000800, 0x00000000, 0x08000000, 0x00000000}, 	// 7  - one_top_lower,
	{6, 0x00000000, 0x00000100, 0x00000000, 0x01000000}, 	// 8  - digit_top_left_a,  digit1_top_a = digit_top_left_a,
	{6, 0x00000000, 0x00000080, 0x00000000, 0x00800000}, 	// 9  - digit_top_left_b,  digit1_top_b = digit_top_left_b,
	{6, 0x00000000, 0x00000040, 0x00000000, 0x00400000}, 	// 10 - digit_top_left_c,  digit1_top_c = digit_top_left_c,
	{6, 0x00000000, 0x00000020, 0x00000000, 0x00200000}, 	// 11 - digit_top_left_d,  digit1_top_d = digit_top_left_d,
	{6, 0x00000000, 0x00000008, 0x00000000, 0x00080000}, 	// 12 - digit_top_left_e,  digit1_top_e = digit_top_left_e,
	{6, 0x00008000, 0x00000000, 0x80000000, 0x00000000}, 	// 13 - digit_top_left_f,  digit1_top_f = digit_top_left_f,
	{6, 0x00001000, 0x00000000, 0x10000000, 0x00000000}, 	// 14 - digit_top_left_g,  digit1_top_g = digit_top_left_g,
	{5, 0x00000000, 0x00000100, 0x00000000, 0x01000000}, 	// 15 - digit_top_right_a, digit0_top_a = digit_top_right_a,
	{5, 0x00000000, 0x00000080, 0x00000000, 0x00800000}, 	// 16 - digit_top_right_b, digit0_top_b = digit_top_right_b,
	{5, 0x00000000, 0x00000040, 0x00000000, 0x00400000}, 	// 17 - digit_top_right_c, digit0_top_c = digit_top_right_c,
	{5, 0x00000000, 0x00000020, 0x00000000, 0x00200000}, 	// 18 - digit_top_right_d, digit0_top_d = digit_top_right_d,
	{5, 0x00000000, 0x00000008, 0x00000000, 0x00080000}, 	// 19 - digit_top_right_e, digit0_top_e = digit_top_right_e,
	{5, 0x00008000, 0x00000000, 0x80000000, 0x00000000}, 	// 20 - digit_top_right_f, digit0_top_f = digit_top_right_f,
	{5, 0x00001000, 0x00000000, 0x10000000, 0x00000000}, 	// 21 - digit_top_right_g, digit0_top_g = digit_top_right_g,
	{0, 0x00001000, 0x00000000, 0x10000000, 0x00000000}, 	// 22 - bar_top_left,
	{0, 0x00000800, 0x00000000, 0x08000000, 0x00000000}, 	// 23 - bar_top_right,
	{3, 0x00000000, 0x00000100, 0x00000000, 0x01000000}, 	// 24 - cycle_nw,
	{3, 0x00000000, 0x00000080, 0x00000000, 0x00800000}, 	// 25 - cycle_ne,
	{3, 0x00000000, 0x00000008, 0x00000000, 0x00080000}, 	// 26 - rocket,
	{3, 0x00000000, 0x00000040, 0x00000000, 0x00400000}, 	// 27 - cycle_sw,
	{3, 0x00000000, 0x00000020, 0x00000000, 0x00200000}, 	// 28 - cycle_se,
	{1, 0x00001000, 0x00000000, 0x10000000, 0x00000000}, 	// 29 - bar_bot_left,
	{1, 0x00000800, 0x00000000, 0x08000000, 0x00000000}, 	// 30 - bar_bot_right,
	{4, 0x00000800, 0x00000000, 0x08000000, 0x00000000}, 	// 31 - one_bot_upper,
	{2, 0x00000800, 0x00000000, 0x08000000, 0x00000000}, 	// 32 - one_bot_lower,
	{4, 0x00000000, 0x00000100, 0x00000000, 0x01000000}, 	// 33 - digit_bot_left_a,  digit1_bot_a = digit_bot_left_a,
	{4, 0x00000000, 0x00000080, 0x00000000, 0x00800000}, 	// 34 - digit_bot_left_b,  digit1_bot_b = digit_bot_left_b,
	{4, 0x00000000, 0x00000040, 0x00000000, 0x00400000}, 	// 35 - digit_bot_left_c,  digit1_bot_c = digit_bot_left_c, ????????
	{4, 0x00000000, 0x00000020, 0x00000000, 0x00200000}, 	// 36 - digit_bot_left_d,  digit1_bot_d = digit_bot_left_d, ????????
	{4, 0x00000000, 0x00000008, 0x00000000, 0x00080000}, 	// 37 - digit_bot_left_e,  digit1_bot_e = digit_bot_left_e, ????????
	{4, 0x00008000, 0x00000000, 0x80000000, 0x00000000}, 	// 38 - digit_bot_left_f,  digit1_bot_f = digit_bot_left_f,
	{4, 0x00001000, 0x00000000, 0x10000000, 0x00000000}, 	// 39 - digit_bot_left_g,  digit1_bot_g = digit_bot_left_g, dim
	{2, 0x00000000, 0x00000100, 0x00000000, 0x01000000}, 	// 40 - digit_bot_right_a, digit0_bot_a = digit_bot_right_a,
	{2, 0x00000000, 0x00000080, 0x00000000, 0x00800000}, 	// 41 - digit_bot_right_b, digit0_bot_b = digit_bot_right_b,
	{2, 0x00000000, 0x00000040, 0x00000000, 0x00400000}, 	// 42 - digit_bot_right_c, digit0_bot_c = digit_bot_right_c,
	{2, 0x00000000, 0x00000020, 0x00000000, 0x00200000}, 	// 43 - digit_bot_right_d, digit0_bot_d = digit_bot_right_d,
	{2, 0x00000000, 0x00000008, 0x00000000, 0x00080000}, 	// 44 - digit_bot_right_e, digit0_bot_e = digit_bot_right_e,
	{2, 0x00008000, 0x00000000, 0x80000000, 0x00000000}, 	// 45 - digit_bot_right_f, digit0_bot_f = digit_bot_right_f,
	{2, 0x00001000, 0x00000000, 0x10000000, 0x00000000}, 	// 46 - digit_bot_right_g, digit0_bot_g = digit_bot_right_g,
	{1, 0x00008000, 0x00000000, 0x80000000, 0x00000000}, 	// 47 - tear_drop,
	{1, 0x00000000, 0x00000100, 0x00000000, 0x01000000}, 	// 48 - arc_bot_1,
	{1, 0x00000000, 0x00000080, 0x00000000, 0x00800000}, 	// 49 - arc_bot_2,
	{1, 0x00000000, 0x00000040, 0x00000000, 0x00400000}, 	// 50 - arc_bot_3,
	{1, 0x00000000, 0x00000020, 0x00000000, 0x00200000}, 	// 51 - arc_bot_4,
	{1, 0x00000000, 0x00000008, 0x00000000, 0x00080000}, 	// 52 - arc_bot_5,	
};



#endif