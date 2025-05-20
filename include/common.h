/*
 * common.h
 *
 *  Created on: May 14, 2025
 *      Author: konstantin
 */

#ifndef COMMON_H_
#define COMMON_H_
#include <stdio.h>
#include <stdint.h>
/*
#ifndef NOSTD

#else
#define printf(...)
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
#endif*/ /* NOSTD */

#define MAX_TASKS (4 + 1) 				// 4 User tasks + 1 Idle
#define IDLE_TASK_ID (0)

#define TASK_STACK_SIZE_B (1024U)
#define SCHEDULER_STACK_SIZE_B (1024U * 2U)
#define TASK_DURATION (1000) // us

// Task definition:
typedef void (*task_handler_t)(void);

typedef enum {
	TASK_READY,
	TASK_BLOCKED
} task_state_t;

typedef struct TCB_ {
	uint32_t *		stack_start;
	uint32_t 		current_state;
	uint32_t 		block_count;
	task_handler_t 	handler;
} TCB_t;


// Basic delay values in scheduler ticks:
#define DELAY_1S (1000U)			// means 1000 scheduler cycles delay
#define DELAY_2S (DELAY_1S * 2)
#define DELAY_4S (DELAY_1S * 4)
#define DELAY_8S (DELAY_1S * 8)

#define ARRAY_SIZE(x) (sizeof(x)/ sizeof(*x))

#endif /* COMMON_H_ */
