/*
 * scheduler.c
 *
 *  Created on: May 14, 2025
 *      Author: konstantin
 */
#include "scheduler.h"
#include "hal_and_isrs.h"
#include "task.h"

/* ======================== DEPENDS ON NEXT HAL FUNCTIONS: ==================================*/
extern void enable_all_configurable_exceptions(void);
extern void schedule(void);
extern void change_sp_to_psp(void);
extern void initial_systick_config(void);
extern void init_scheduler_stack(void *start_of_stack);
extern void init_task_stack(TCB_t *task_descriptor);

/* ======================== GLOBAL STATE ==================================*/
static uint32_t global_tick_count = 0;

static TCB_t tasks[MAX_TASKS] = {
		{(uint32_t *)TASK_IDLE_STACK_START, TASK_READY, 0, task_idle},
		{(uint32_t *)TASK_1_STACK_START, TASK_READY, 0, task_1_handler},
		{(uint32_t *)TASK_2_STACK_START, TASK_READY, 0, task_2_handler},
		{(uint32_t *)TASK_3_STACK_START, TASK_READY, 0, task_3_handler},
		{(uint32_t *)TASK_4_STACK_START, TASK_READY, 0, task_4_handler}

};

uint32_t current_task = 1;

/* ========================================================================*/

static void init_tasks(uint32_t n_tasks)
{
	for (int i = 0; i < n_tasks; i++) {
		tasks[i].current_state = TASK_READY;
		init_task_stack(&tasks[i]);
	}
}

/**
 * @brief Main function: initialize:
 *                       - System Fault exception handlers
 *                       - SysTick timer and PendSV (context switch) handlers
 *                       - 4 user tasks and runs them in Thread mode starting from task 1.
 */
void init_and_run_scheduler(void)
{
	enable_all_configurable_exceptions();
	init_scheduler_stack((uint32_t *)SCHEDULER_STACK_START);
	init_tasks(MAX_TASKS);
	initial_systick_config();
	change_sp_to_psp();
	current_task = 1;
	task_1_handler();

	// Should never come here!!!
	// Can't exit from this function to MAIN because SP was changed from MSP to PSP, so
	// return will cause stack corruption or fault.
}

/**
 * @brief     Sleep for requested scheduler ticks
 * @param[in] tick_count - number of scheduler ticks. Each tick equals to TASK_DURAION time.
 */
void delay_task(uint32_t tick_count) {
	INTERRUPT_DISABLE();	// Disable interrupts because current task and tasks are global and next modification
							// should be atomic
	if (current_task != IDLE_TASK_ID) {
		// Block current task:
		tasks[current_task].block_count = global_tick_count + tick_count;
		tasks[current_task].current_state = TASK_BLOCKED;
		// Trigger scheduler:
		schedule();
	}
	INTERRUPT_ENABLE();
}

/**
 * @brief     Increment scheduler tick.
 */
void update_global_tick_count(void) {
	global_tick_count++;
}

/**
 * @brief     Check if any of tasks should be unlocked on current scheduler tick.
 */
void update_blocked_tasks(void) {
	for (int i = 1; i < MAX_TASKS; i++) // Skip idle task
	{
		if (tasks[i].current_state == TASK_BLOCKED)
		{
			if (tasks[i].block_count == global_tick_count)
			{
				tasks[i].block_count = 0;
				tasks[i].current_state = TASK_READY;
			}
		}
	}
}

/**
 * @brief     Get PSP stack pointer of currently running task
 * @return    stack pointer of currently running (just before exception) task.
 */
uint32_t *get_psp_of_current_task(void)
{
	return tasks[current_task].stack_start;
}

/**
 * @brief     Save PSP stack pointer of currently running task
 * param[in]  psp_val - stack pointer of currently running (just before exception) task.
 */
void save_psp_value(uint32_t *psp_val)
{
	tasks[current_task].stack_start = psp_val;
}

/**
 * @brief     Runs next task selection. If all tasks are in TASK_BLOCKED state, then task_idle runs.
 */
void update_to_next_task(void)
{
	uint32_t task_selected = 0;
	// Loop over tasks till TASK_READY is found
	for (int i = 0; i < MAX_TASKS; i++) {
		current_task++;
		current_task %= MAX_TASKS; // FIXME!!!
		if (current_task != IDLE_TASK_ID && tasks[current_task].current_state == TASK_READY) {
			task_selected = 1;
			break;
		}
	}
	// If no TASK_READY is found, choose IDLE task
	if (task_selected == 0)
		current_task = IDLE_TASK_ID;
}


