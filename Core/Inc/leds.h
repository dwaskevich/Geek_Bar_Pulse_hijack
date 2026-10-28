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

#define NUM_LED_CATHODES		(7)
#define NUM_DIGITS					(3)

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

/* enum to define LED groups */
enum 
{
	one_top,
	digit_top_left,
	digit_top_right,
	one_bot,
	digit_bot_left,
	digit_bot_right,
	arc_top,
	arc_bottom,
	cycle,
	bar_top,
	bar_bottom,
	slice_1,
	slice_2,
	slice_3,
	slice_4,
	slice_5,
	slice_6,
	slice_7,
	slice_8,
	slice_9,
	slice_10,
	slice_11,
	slice_12,
	slice_13,
	slice_14,
	slice_15,
	slice_16,
	slice_17,
	slice_18,
	slice_19,
	slice_20,
	slice_21,
	slice_22,
	slice_23,
	NUM_GROUPS
};

/* enum to define 7-segment displays */
enum 
{
	display_top,
	display_bottom,
	NUM_DISPLAYS
};

typedef struct
{
	uint8_t  scan_line;
	uint32_t set_segment_GPIOA;
	uint32_t set_segment_GPIOB;
} LED_port_position_t;

extern const LED_port_position_t LED_port_position[MAX_LEDS];
extern const uint8_t led_group_array[NUM_GROUPS][8];
extern const uint8_t seven_segment_displays[NUM_DISPLAYS][3];

#endif