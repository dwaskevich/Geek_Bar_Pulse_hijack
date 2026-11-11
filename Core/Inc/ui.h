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

typedef enum
{
	fast = 110,
	medium = 220,
	slow = 440
} rotate_cycle_speed_t;

void heartbeat(taskId_t id);
void slide_switch_process_events(taskId_t id);
void rotate_cycle_cw(taskId_t id);
void rotate_cycle_ccw(taskId_t id);
void rotate_cycle_alternate(taskId_t id);
void post_battery_status(taskId_t id);
void post_battery_voltage(taskId_t id);

#endif