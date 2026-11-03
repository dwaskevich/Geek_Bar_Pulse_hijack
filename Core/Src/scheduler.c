/*
 * scheduler.c
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
 #include "scheduler.h"
 
 #define MAX_TASKS 8
 
 typedef struct {
    TaskCallback callback;
    uint32_t interval_ms;
    uint32_t last_run;
    uint8_t priority;
} task_t;
 
static task_t task_list[MAX_TASKS];
static uint8_t task_count = 0;

void scheduler_init()
{
	task_count = 0;	
}

taskId_t scheduler_register_task(TaskCallback callback, uint32_t interval_ms, uint8_t priority)
{
	if(task_count < MAX_TASKS)
	{
		uint8_t task_id = task_count;
		task_list[task_count++] = (task_t){callback, interval_ms, 0, priority};
		return task_id;
	}	
	return 0xff;
}

void scheduler_run_tasks(void)
{
	uint32_t now = HAL_GetTick();
	
	for (uint8_t i = 0; i < task_count; i++)
	{
		if(0 != task_list[i].interval_ms && (now - task_list[i].last_run) >= task_list[i].interval_ms)
		{
			task_list[i].last_run = now;
			task_list[i].callback();
		}
	}	
}

void scheduler_reschedule_task(uint8_t task_id, uint32_t interval_ms, uint8_t priority)
{
	(void) priority;
	if(task_id < task_count)
	{
		task_list[task_id].interval_ms = interval_ms;		
	}	
}


