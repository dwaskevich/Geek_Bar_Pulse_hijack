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

#include <stdbool.h>
#include "ui.h"
#include "display.h"
#include "leds.h"
#include "battery.h"
#include "slide_switch.h"
#include "low_power.h"
#include "scheduler.h"

#define DEBOUNCE_TIME_MS	50

battery_status_t battery_status;
extern uint32_t inactivity_timestamp;

void heartbeat(taskId_t id)
{
	(void) id;
	display_led_toggle(rocket);
}

void slide_switch_process_events(taskId_t id)
{
	(void) id;
	static bool debounce_event = true;
	static slide_switch_position_t position, previous_position;
	
	if(previous_position != slide_switch_get_position()) /* only process if switch position has changed */
	{
		if(true == debounce_event) /* if this is a start event, record position and reschedule task for debounce time */
		{
			position = slide_switch_get_position(); /* record current position of slide switch for debounce */
			scheduler_reschedule_task(id, DEBOUNCE_TIME_MS, DEFAULT_PRIORITY); /* reschedule task */
			debounce_event = false;
			return;
		}
		else /* debounce window expired ... validate switch event */
		{
			if(position == slide_switch_get_position()) /* if true, position hasn't changed (position validated) */
			{
				inactivity_timestamp = HAL_GetTick(); /* reset inactivity timeout */
				previous_position = position; /* remember current position */
				switch (position)
				{
					case SWITCH_CLOSED:
						display_group_fill(arc_bottom, SWITCH_CLOSED + 1);
						enter_low_power(low_power_enable_flag, true);
						scheduler_resume_all_tasks();
						break;
					case SWITCH_NORMAL:
						display_group_fill(arc_bottom, SWITCH_NORMAL + 1);
						break;
					case SWITCH_PULSE:
						display_group_fill(arc_bottom, SWITCH_PULSE + 1);
						run_test_patterns();
						display_group_fill(arc_bottom, SWITCH_PULSE + 1);
						break;
					
					default:
						break;
				}
				scheduler_reschedule_task(id, 2, DEFAULT_PRIORITY); /* re-que the task for quick response */
			}
			debounce_event = true; /* reset the debounce (i.e. start-of-event) flag */
		}
	}
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
	static bool most_significant = true;
	
	if(most_significant)
	{
		display_led(bar_bot_right, 0);
		display_led(bar_bot_left, 1);
		display_decimal(display_bottom, battery_status.millivolts / 100);
		most_significant = false;
		scheduler_reschedule_task(id, 750, DEFAULT_PRIORITY);
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
		scheduler_reschedule_task(id, 1250, DEFAULT_PRIORITY);
		most_significant = true;
	}
	
}