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
 
 #include <stdbool.h>
 #include "main.h"
 #include "scheduler.h"
 
 #define MAX_TASKS 16
 
// typedef struct {
//    TaskCallback_t callback;
//		bool suspend_task_flag;
//    uint32_t interval_ms; /* value of 0 (TASK_DISABLED) prevents task from running (i.e. task is registered but not running) */
//    uint32_t last_run;
//    uint8_t priority;
//} task_t;

//extern taskId_t heartbeat_task_id, slide_switch_task_id, rotate_cycle_cw_task_id, rotate_cycle_ccw_task_id, \
//		rotate_cycle_alternate_task_id, battery_status_task_id, battery_voltage_task_id, run_test_patterns_task_id;
 
static task_t task_list[MAX_TASKS];
static uint8_t task_count = 0;

void scheduler_init()
{
	task_count = 0;	
}

taskId_t scheduler_register_task(TaskCallback_t callback, uint32_t interval_ms, uint8_t priority)
{
	if(task_count < MAX_TASKS)
	{
		uint8_t task_id = task_count;
		task_list[task_count++] = (task_t){callback, 0, interval_ms, 0, priority};
		return task_id;
	}	
	return 0xff;
}

void scheduler_run_tasks(void)
{
	uint32_t now = HAL_GetTick();
	
	for (uint8_t i = 0; i < task_count; i++)
	{
		if(false == task_list[i].suspend_task_flag && TASK_DISABLED != task_list[i].interval_ms && (now - task_list[i].last_run) >= task_list[i].interval_ms)
		{
			task_list[i].last_run = now; /* refresh last run time */
			task_list[i].callback(i); /* dispatch callback */
		}
	}
}

void scheduler_reschedule_task(uint8_t task_id, uint32_t interval_ms, uint8_t priority)
{
	if(task_id < task_count)
	{
		task_list[task_id].interval_ms = interval_ms;
		task_list[task_id].priority = priority;
	}	
}

void scheduler_run_once(uint8_t task_id)
{
	(void) task_id;
	
	
}

void scheduler_suspend_task(uint8_t task_id)
{
	if(task_id < task_count)
	{
		task_list[task_id].suspend_task_flag = true;
	}	
}

void scheduler_resume_task(uint8_t task_id)
{
	if(task_id < task_count)
	{
		task_list[task_id].suspend_task_flag = false;
		task_list[task_id].last_run = HAL_GetTick();
	}	
}

void scheduler_suspend_all_tasks(void)
{
	for(uint8_t i = 0; i < task_count; i++)
	{
		task_list[i].suspend_task_flag = true;
	}	
}

void scheduler_resume_all_tasks(void)
{
	uint32_t now = HAL_GetTick();
	for(uint8_t i = 0; i < task_count; i++)
	{
		task_list[i].suspend_task_flag = false;
		task_list[i].last_run = now;
	}	
}

