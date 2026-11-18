/*
 * ui.h
 *
 *  Created:	2-Nov-2026
 *  Author:		David Waskevich
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UI_H
#define __UI_H

#include "scheduler.h"

#define SET_RED_COMPARE(x)  	(__HAL_TIM_SET_COMPARE(&htim3, RED_CHANNEL, x))
#define SET_GREEN_COMPARE(x)  (__HAL_TIM_SET_COMPARE(&htim3, GREEN_CHANNEL, x))
#define SET_BLUE_COMPARE(x)  	(__HAL_TIM_SET_COMPARE(&htim3, BLUE_CHANNEL, x))

typedef enum
{
	FAST = 110,
	MEDIUM = 220,
	SLOW = 440
} rotate_cycle_speed_t;

uint8_t ui_init(void);
void ui_enter_low_power(bool enable, bool fade_out);
void ui_wakeup(void);
void heartbeat(taskId_t id);
void slide_switch_process_events(taskId_t id);
void rotate_cycle_cw(taskId_t id);
void rotate_cycle_ccw(taskId_t id);
void rotate_cycle_alternate(taskId_t id);
void post_battery_status(taskId_t id);
void post_battery_voltage(taskId_t id);
void run_test_patterns(taskId_t id);

#endif