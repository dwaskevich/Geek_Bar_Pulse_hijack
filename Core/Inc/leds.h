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

/* enum to identify and define individual LEDs (from top to bottom) */
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
	max_leds
};

typedef struct
{
	uint8_t  scan_line;
	uint32_t segment_GPIOA;
	uint32_t segment_GPIOB;
} LED_port_position_t;

const static LED_port_position_t LED_port_position[max_leds] =
{
/*{scan, PortA,   PortB} */
	{0, 0x00000000, 0x00000100}, 	// arc_top_1,
	{0, 0x00000000, 0x00000080}, 	// arc_top_2,
	{0, 0x00000000, 0x00000040}, 	// arc_top_3,
	{0, 0x00000000, 0x00000020}, 	// arc_top_4,
	{0, 0x00000000, 0x00000008}, 	// arc_top_5,
	{0, 0x00008000, 0x00000000}, 	// lightning_bolt,
	{6, 0x00000800, 0x00000000}, 	// one_top_upper,
	{5, 0x00000800, 0x00000000}, 	// one_top_lower,
	{6, 0x00000000, 0x00000100}, 	// digit_top_left_a,  digit1_top_a = digit_top_left_a,
	{6, 0x00000000, 0x00000080}, 	// digit_top_left_b,  digit1_top_b = digit_top_left_b,
	{6, 0x00000000, 0x00000040}, 	// digit_top_left_c,  digit1_top_c = digit_top_left_c,
	{6, 0x00000000, 0x00000020}, 	// digit_top_left_d,  digit1_top_d = digit_top_left_d,
	{6, 0x00000000, 0x00000008}, 	// digit_top_left_e,  digit1_top_e = digit_top_left_e,
	{6, 0x00008000, 0x00000000}, 	// digit_top_left_f,  digit1_top_f = digit_top_left_f,
	{6, 0x00001000, 0x00000000}, 	// digit_top_left_g,  digit1_top_g = digit_top_left_g,
	{5, 0x00000000, 0x00000100}, 	// digit_top_right_a, digit0_top_a = digit_top_right_a,
	{5, 0x00000000, 0x00000080}, 	// digit_top_right_b, digit0_top_b = digit_top_right_b,
	{5, 0x00000000, 0x00000040}, 	// digit_top_right_c, digit0_top_c = digit_top_right_c,
	{5, 0x00000000, 0x00000020}, 	// digit_top_right_d, digit0_top_d = digit_top_right_d,
	{5, 0x00000000, 0x00000008}, 	// digit_top_right_e, digit0_top_e = digit_top_right_e,
	{5, 0x00008000, 0x00000000}, 	// digit_top_right_f, digit0_top_f = digit_top_right_f,
	{5, 0x00001000, 0x00000000}, 	// digit_top_right_g, digit0_top_g = digit_top_right_g,
	{0, 0x00001000, 0x00000000}, 	// bar_top_left,
	{0, 0x00000800, 0x00000000}, 	// bar_top_right,
	{3, 0x00000000, 0x00000100}, 	// cycle_nw,
	{3, 0x00000000, 0x00000080}, 	// cycle_ne,
	{3, 0x00000000, 0x00000008}, 	// rocket,
	{3, 0x00000000, 0x00000040}, 	// cycle_sw,
	{3, 0x00000000, 0x00000020}, 	// cycle_se,
	{1, 0x00001000, 0x00000000}, 	// bar_bot_left,
	{1, 0x00000800, 0x00000000}, 	// bar_bot_right,
	{4, 0x00000800, 0x00000000}, 	// one_bot_upper,
	{2, 0x00000800, 0x00000000}, 	// one_bot_lower,
	{4, 0x00000000, 0x00000100}, 	// digit_bot_left_a,  digit1_bot_a = digit_bot_left_a,
	{4, 0x00000000, 0x00000080}, 	// digit_bot_left_b,  digit1_bot_b = digit_bot_left_b,
	{4, 0x00000000, 0x00000040}, 	// digit_bot_left_c,  digit1_bot_c = digit_bot_left_c, ????????
	{4, 0x00000000, 0x00000020}, 	// digit_bot_left_d,  digit1_bot_d = digit_bot_left_d, ????????
	{4, 0x00000000, 0x00000008}, 	// digit_bot_left_e,  digit1_bot_e = digit_bot_left_e, ????????
	{4, 0x00008000, 0x00000000}, 	// digit_bot_left_f,  digit1_bot_f = digit_bot_left_f,
	{4, 0x00001000, 0x00000000}, 	// digit_bot_left_g,  digit1_bot_g = digit_bot_left_g, dim
	{2, 0x00000000, 0x00000100}, 	// digit_bot_right_a, digit0_bot_a = digit_bot_right_a,
	{2, 0x00000000, 0x00000080}, 	// digit_bot_right_b, digit0_bot_b = digit_bot_right_b,
	{2, 0x00000000, 0x00000040}, 	// digit_bot_right_c, digit0_bot_c = digit_bot_right_c,
	{2, 0x00000000, 0x00000020}, 	// digit_bot_right_d, digit0_bot_d = digit_bot_right_d,
	{2, 0x00000000, 0x00000008}, 	// digit_bot_right_e, digit0_bot_e = digit_bot_right_e,
	{2, 0x00008000, 0x00000000}, 	// digit_bot_right_f, digit0_bot_f = digit_bot_right_f,
	{2, 0x00001000, 0x00000000}, 	// digit_bot_right_g, digit0_bot_g = digit_bot_right_g,
	{1, 0x00008000, 0x00000000}, 	// tear_drop,
	{1, 0x00000000, 0x00000100}, 	// arc_bot_1,
	{1, 0x00000000, 0x00000080}, 	// arc_bot_2,
	{1, 0x00000000, 0x00000040}, 	// arc_bot_3,
	{1, 0x00000000, 0x00000020}, 	// arc_bot_4,
	{1, 0x00000000, 0x00000008}, 	// arc_bot_5,	
};



#endif