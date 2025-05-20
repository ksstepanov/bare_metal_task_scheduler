/*
 * task.h
 *
 *  Created on: May 14, 2025
 *      Author: konstantin
 */

#ifndef TASK_H_
#define TASK_H_

#include "common.h"

/**
 * @brief Idle task runs when all other taks are in TASK_BLOCKED state
 */
void task_idle(void);

/**
 * @brief User task handler. Can be populated with anything.
 */
void task_1_handler(void);

/**
 * @brief User task handler. Can be populated with anything.
 */
void task_2_handler(void);

/**
 * @brief User task handler. Can be populated with anything.
 */
void task_3_handler(void);

/**
 * @brief User task handler. Can be populated with anything.
 */
void task_4_handler(void);

#endif /* TASK_H_ */
