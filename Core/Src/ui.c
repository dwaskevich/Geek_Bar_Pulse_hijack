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

#include "main.h"
#include "ui.h"
#include "display.h"
#include "leds.h"
#include "battery.h"

battery_status_t battery_status;

void heartbeat(taskId_t id)
{
	(void) id;
	display_led_toggle(tear_drop);
}

void rotate_cycle_cw(taskId_t id)
{
	(void) id;
	static uint8_t cycle_segment = 0;
	display_group(cycle, 1 << (cycle_segment++ % led_group_array[cycle][0]));
}

void rotate_cycle_ccw(taskId_t id)
{
	(void) id;
	static uint8_t cycle_segment = 0;
	display_group(cycle, 8 >> (cycle_segment++ % led_group_array[cycle][0]));
}

void rotate_cycle_alternate(taskId_t id)
{
	(void) id;
	static uint8_t cycle_segment = 0x05;
	display_group(cycle, cycle_segment);
	cycle_segment = ~cycle_segment;
}

void post_battery_status(taskId_t id)
{
	(void) id;
	/* get battery status and update display */
	battery_status = battery_get_status();
	display_led(lightning_bolt, 1);
	display_decimal(display_top, battery_status.percent);		
	display_group_fill(arc_top, battery_status.bars);
}

void post_battery_voltage(taskId_t id)
{
	/* get battery voltage and update bottom display */
	static _Bool most_significant = 1;
	
	if(most_significant)
	{
		display_led(bar_bot_right, 0);
		display_led(bar_bot_left, 1);
		display_decimal(display_bottom, battery_status.millivolts / 100);
		most_significant = 0;
		scheduler_reschedule_task(id, 750, 0);
		return;
	}
	else
	{
		display_led(bar_bot_left, 0);
		display_led(bar_bot_right, 1);
		if(battery_status.millivolts % 100 < 10)
		{
			display_seven_segment(digit_bot_left, 0);
			display_seven_segment(digit_bot_right, battery_status.millivolts % 100);
		}
		else
			display_decimal(display_bottom, battery_status.millivolts % 100);
		scheduler_reschedule_task(id, 1250, 0);
		most_significant = 1;
	}
	
}