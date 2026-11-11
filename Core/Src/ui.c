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
#include "gpio_defines.h"

extern volatile _Bool slide_switch_event;
battery_status_t battery_status;

void heartbeat(taskId_t id)
{
	(void) id;
	display_led_toggle(rocket);
}

void slide_switch_process_events(taskId_t id)
{
	static _Bool debounce_event_flag = 1;
	static _Bool slide_switch_state;
	
	if(!slide_switch_event) /* just return if no event */
		return;
	
	/* record current state of switch for debounce */
	slide_switch_state = HAL_GPIO_ReadPin(SLIDE_SWITCH_PORT, SLIDE_SWITCH_PIN);
	
	if(debounce_event_flag) /* if this is a start event, reschedule task for debounce time */
	{
		scheduler_reschedule_task(id, 20, 0);
		debounce_event_flag = 0;
		return;
	}
	else /* debounce window expired ... validate switch event */
	{
		if(slide_switch_state == HAL_GPIO_ReadPin(SLIDE_SWITCH_PORT, SLIDE_SWITCH_PIN))
		{
			if(0 == slide_switch_state) /* falling edge detected */
			{
				display_led(tear_drop, 1);
				display_led(bar_top_left, 0);
			}
			else /* rising edge detected */
			{
				display_led(bar_top_left, 1);
				display_led(tear_drop, 0);
			}
			scheduler_reschedule_task(id, 2, 0); /* re-que the task for quick response */
		}
		debounce_event_flag = 1; /* reset the debounce (i.e. start of event) flag */
		slide_switch_event = 0; /* clear the volatile flag */
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