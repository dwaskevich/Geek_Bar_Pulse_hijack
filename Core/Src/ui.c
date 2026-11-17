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
#include "main.h"
#include "ui.h"
#include "display.h"
#include "leds.h"
#include "battery.h"
#include "slide_switch.h"
#include "low_power.h"
#include "scheduler.h"

#define DEBOUNCE_TIME_MS	50

//#define SET_RED_COMPARE(x)  	(__HAL_TIM_SET_COMPARE(&htim3, RED_CHANNEL, x))
//#define SET_GREEN_COMPARE(x)  (__HAL_TIM_SET_COMPARE(&htim3, GREEN_CHANNEL, x))
//#define SET_BLUE_COMPARE(x)  	(__HAL_TIM_SET_COMPARE(&htim3, BLUE_CHANNEL, x))

#define PATTERN_DELAY				(50)
#define GROUP_DELAY					(100)

battery_status_t battery_status;
extern uint32_t inactivity_timestamp;

extern taskId_t run_test_patterns_task_id, heartbeat_task_id, battery_status_task_id, battery_voltage_task_id, \
		rotate_cycle_cw_task_id, rotate_cycle_alternate_task_id;

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
						scheduler_reschedule_task(run_test_patterns_task_id, TASK_DISABLED, DEFAULT_PRIORITY);
						display_group_fill(arc_bottom, SWITCH_CLOSED + 1);
						enter_low_power(low_power_enable_flag, true);
//						scheduler_resume_all_tasks();
						break;
					case SWITCH_NORMAL:
						scheduler_resume_all_tasks();
						scheduler_reschedule_task(run_test_patterns_task_id, TASK_DISABLED, DEFAULT_PRIORITY);
						display_group_fill(arc_bottom, SWITCH_NORMAL + 1);
						break;
					case SWITCH_PULSE:
						display_group_fill(arc_bottom, SWITCH_PULSE + 1);
//						run_test_patterns();
						scheduler_suspend_task(heartbeat_task_id);
						scheduler_suspend_task(rotate_cycle_cw_task_id);
						scheduler_suspend_task(rotate_cycle_alternate_task_id);
						scheduler_suspend_task(battery_status_task_id);
						scheduler_suspend_task(battery_voltage_task_id);
						scheduler_reschedule_task(run_test_patterns_task_id, 10, DEFAULT_PRIORITY);
						scheduler_resume_task(run_test_patterns_task_id);
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

void run_test_patterns(taskId_t id)
{
	static uint8_t sequence_id = 0;
	
	/* animate peripheral RGB LEDs */
	HAL_TIM_PWM_Start(&htim3, RED_CHANNEL);
	HAL_TIM_PWM_Start(&htim3, GREEN_CHANNEL);
	HAL_TIM_PWM_Start(&htim3, BLUE_CHANNEL);
	
//	SET_RED_COMPARE(1023);
//	HAL_Delay(ANIMATION_DELAY);
//	SET_BLUE_COMPARE(1023);
//	HAL_Delay(ANIMATION_DELAY);
//	SET_GREEN_COMPARE(1023);
//	HAL_Delay(ANIMATION_DELAY);
//	SET_RED_COMPARE(0);
//	HAL_Delay(ANIMATION_DELAY);
//	SET_BLUE_COMPARE(0);
//	HAL_Delay(ANIMATION_DELAY);
//	SET_GREEN_COMPARE(0);
//	HAL_Delay(ANIMATION_DELAY);
	
	inactivity_timestamp = HAL_GetTick();
	
	switch (sequence_id)
  {
  	case 0:
			display_clear(); /* clear display */			
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 1;
  		break;
		
  	case 1:
			/* testing led array ... */
			SET_RED_COMPARE(1023);		
			/* illuminate each display elements and leave trail */
			for(uint8_t i = 0; i < MAX_LEDS; i++)
			{
				display_led(i, 1);
				HAL_Delay(PATTERN_DELAY);
			}
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 2;
  		break;
		
		/* pick up trail of display elements (in reverse order) */
  	case 2:
			for(uint8_t i = 0; i < MAX_LEDS; i++)
			{
				display_led(MAX_LEDS - 1 - i, 0);
				HAL_Delay(PATTERN_DELAY);
			}
			SET_RED_COMPARE(0);
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 3;
  		break;
		
		/* change border color */
  	case 3:
			SET_GREEN_COMPARE(1023);
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 5;
  		break;
		
		/* flash display 3 times */
  	case 4:
			display_all();
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 5;
  		break;
		
  	case 5:
			display_clear();
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 6;
  		break;
		
  	case 6:
			display_all();
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 7;
  		break;
		
  	case 7:
			display_clear();
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 8;
  		break;
		
  	case 8:
			display_all();
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 9;
  		break;
		
  	case 9:
			display_clear();
			scheduler_reschedule_task(id, 500, DEFAULT_PRIORITY);
			sequence_id = 10;
  		break;
		
		/* testing groups ... leave trail */
  	case 10:
			for(uint8_t i = one_top; i <= bar_bottom; i++)
			{
				for(uint8_t j = 0; j < led_group_array[i][0]; j++)
				{
					display_led(led_group_array[i][j + 1], 1);
					HAL_Delay(PATTERN_DELAY);
				}
				HAL_Delay(250);
			}
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 11;
  		break;
		
		/* testing groups ... pick up trail (in reverse order) */
  	case 11:
			for(uint8_t i = one_top; i <= bar_bottom; i++)
			{			
				for(uint8_t j = 0; j < led_group_array[bar_bottom - i][0]; j++)
				{
					display_led(led_group_array[bar_bottom - i][led_group_array[bar_bottom - i][0] - 1 - j + 1], 0);
					HAL_Delay(PATTERN_DELAY);
				}
				HAL_Delay(250);
			}
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 12;
  		break;
		
  	/* testing groups with display_group() */
  	case 12:
			for(uint8_t i = one_top; i <= bar_bottom; i++)
			{
				display_group(i, 0xff);
				HAL_Delay(GROUP_DELAY);
			}
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 13;
  		break;
		
  	/* testing groups with display_group() */
  	case 13:
			for(uint8_t i = one_top; i <= bar_bottom; i++)
			{
				display_group(i, 0x00);
				HAL_Delay(GROUP_DELAY);
			}
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 14;
  		break;
		
		/* testing decimal 7-segment displays */
  	case 14:
			SET_GREEN_COMPARE(0);
			for(uint8_t count = 0; count <= 199; count++)
			{
				SET_BLUE_COMPARE(count * 5);
				display_decimal(display_top, count);
				display_decimal(display_bottom, 199 - count);
				HAL_Delay(GROUP_DELAY);
			}
			SET_BLUE_COMPARE(1023);
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			inactivity_timestamp = HAL_GetTick();
			sequence_id = 15;
  		break;
		
		case 15:
			SET_BLUE_COMPARE(0);
			display_clear();
			scheduler_reschedule_task(id, 250, DEFAULT_PRIORITY);
			sequence_id = 16;
  		break;
		
		case 16:
			scheduler_reschedule_task(id, TASK_DISABLED, DEFAULT_PRIORITY);
			scheduler_resume_all_tasks();
			sequence_id = 0;
  		break;
  	
  	default:
  		break;
  }	
}