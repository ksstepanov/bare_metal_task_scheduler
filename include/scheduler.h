/*
 * scheduler.h
 *
 *  Created on: May 14, 2025
 *      Author: konstantin
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_
#include "common.h"

/* ================== Scheduler user API ===================================== */
/**
 * @brief Main function: initialize:
 *                       - System Fault exception handlers
 *                       - SysTick timer and PendSV (context switch) handlers
 *                       - 4 user tasks and runs them in Thread mode starting from task 1.
 */
void init_and_run_scheduler(void);

/**
 * @brief     Sleep for requested scheduler ticks
 * @param[in] tick_count - number of scheduler ticks. Each tick equals to TASK_DURAION time.
 */
void delay_task(uint32_t tick_count);

/* ================== Service API calls used by HAL: ========================== */
/**
 * @brief     Get PSP stack pointer of currently running task
 * @return    stack pointer of currently running (just before exception) task.
 */
uint32_t *get_psp_of_current_task(void);

/**
 * @brief     Save PSP stack pointer of currently running task
 * param[in]  psp_val - stack pointer of currently running (just before exception) task.
 */
void save_psp_value(uint32_t *psp_val);

/**
 * @brief     Runs next task selection. If all tasks are in TASK_BLOCKED state, then task_idle runs.
 */
void update_to_next_task(void);

/**
 * @brief     Check if any of tasks should be unlocked on current scheduler tick.
 */
void update_blocked_tasks(void);

/**
 * @brief     Increment scheduler tick.
 */
void update_global_tick_count(void);

#endif /* SCHEDULER_H_ */
