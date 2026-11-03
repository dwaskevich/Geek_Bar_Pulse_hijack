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

typedef void (*TaskCallback)(void);
typedef uint8_t taskId_t;

void scheduler_init(void);
taskId_t scheduler_register_task(TaskCallback callback, uint32_t interval_ms, uint8_t priority);
void scheduler_run_tasks(void);
void scheduler_reschedule_task(uint8_t task_id, uint32_t interval_ms, uint8_t priority);


#endif