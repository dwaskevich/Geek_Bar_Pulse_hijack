/*
 * scheduler.h
 *
 *  Created:	2-Nov-2026
 *  Author:		David Waskevich
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include <stdint.h>

typedef uint8_t taskId_t;
typedef void (*TaskCallback_t)(taskId_t id);

void scheduler_init(void);
taskId_t scheduler_register_task(TaskCallback_t callback, uint32_t interval_ms, uint8_t priority);
void scheduler_run_tasks(void);
void scheduler_reschedule_task(uint8_t task_id, uint32_t interval_ms, uint8_t priority);
void scheduler_run_once(uint8_t task_id);
void scheduler_suspend_task(uint8_t task_id);
void scheduler_suspend_all_tasks(void);

#endif