/*
 * ui.c
 *
 *  Created:	2-Nov-2026
 *  Author:		David Waskevich
 *
 *  Description: 
 *
 *	Update 2-Nov-2026:
 *			- created file
 *  
 */

#include "ui.h"
#include "display.h"
#include "leds.h"

void heartbeat(void)
{
	display_led_toggle(lightning_bolt);
}

void rotate_cycle_cw(void)
{
	static uint8_t cycle_segment = 0;
	display_group(cycle, 1 << (cycle_segment++ % led_group_array[cycle][0]));
}

void rotate_cycle_ccw(void)
{
	static uint8_t cycle_segment = 0;
	display_group(cycle, 8 >> (cycle_segment++ % led_group_array[cycle][0]));
}