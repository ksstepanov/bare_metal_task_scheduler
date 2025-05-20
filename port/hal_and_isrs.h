/*
 * isrs_and_ex_handlers.h
 *
 *  Created on: May 14, 2025
 *      Author: konstantin
 */

#ifndef HAL_AND_ISRS_H_
#define HAL_AND_ISRS_H_
#include "common.h"

#define CPU_CLOCK_RATE (16 * 1000000) 	// 16 MHz, internal processor clock used (HSI)
#define SRAM_START (0x20000000)
#define SRAM_SIZE (256U * 1024U)
#define SRAM_END (SRAM_START + SRAM_SIZE) //20040000

// Num tasks should match MAX_TASKS in common.h:
#define TASK_1_STACK_START (SRAM_END - 0 * TASK_STACK_SIZE_B)
#define TASK_2_STACK_START (SRAM_END - 1 * TASK_STACK_SIZE_B)
#define TASK_3_STACK_START (SRAM_END - 2 * TASK_STACK_SIZE_B)
#define TASK_4_STACK_START (SRAM_END - 3 * TASK_STACK_SIZE_B)
#define TASK_IDLE_STACK_START (SRAM_END - 4 * TASK_STACK_SIZE_B)
#define SCHEDULER_STACK_START (SRAM_END - 5 * TASK_STACK_SIZE_B)

/* ============= SCB (System Control Block ================ */
// FAULT regs:
#define SCB_USFR (0xE000ED2A)
#define SCB_BSFR (0xE000ED29)
#define SCB_MMFSR (0xE000ED28)
#define SCB_HFSR (0xE000ED2C)

// Address return registers:
#define SCB_MMAR (0xE000ED34)
#define SCB_BFAR (0xE000ED38)

// System Control Block(SCB) control register
# define SCB_SHCSR (0xE000ED24)
# define SCB_USAGE_FAULT_EN_BIT (18)
# define SCB_MEMMANAGE_FAULT_EN_BIT (16)
# define SCB_BUS_FAULT_EN_BIT (17)

// System Control Block - Interrupt Control Status Register
#define SCB_ICSR (0xE000ED04)
#define SCB_ICSR_PEND_SV_EN_BIT (28)

/* ============= SysTick - System Timer =================== */
// CSR - Control State Register:
#define SYSTICK_CSR (0xE000E010)
#define SYSTICK_CSR_ENABLE_BIT (0)
#define SYSTICK_CSR_ENABLE_INTERRUPT_BIT (1)
#define SYSTICK_CSR_CLKSOURCE_BIT (2)

// RVR - Reset Value Register:
#define SYSTICK_RVR (0xE000E014)
#define SYSTICK_RESET_VAL ((CPU_CLOCK_RATE / TASK_DURATION) - 1) // -1 because the exception happens when switching from 0 to RESET_VAL

/* =========================================================*/

// Init values of general registers for tasks:
#define TINIT_PSR_VAL (1 << 24) // T bit should be 1
#define TINIT_LR_VAL (0xFFFFFFFD) // return to thread mode and use PSP
#define TINIT_GEN_PURP_REG_VAL (0)

#define CONTEXT_TOTAL_REGS (16)
#define CONTEXT_GP_REGS (13)	//R0 - R12

// Implementation of scheduler calls:
// Use PRIMASK register to disable all interrupts for critical sections:
#define INTERRUPT_DISABLE() do {__asm volatile ("MOV R0, #0x01"); __asm volatile ("MSR PRIMASK, R0");} while(0);
#define INTERRUPT_ENABLE() do {__asm volatile ("MOV R0, #0x0"); __asm volatile ("MSR PRIMASK, R0");} while(0);

/**
 * @brief Enable all System Fault handlers (Usage, Memory, Bus)
 */
void enable_all_configurable_exceptions(void);

/**
 * @brief Force trigger scheduler (PendSV Handler for context switching)
 */
void schedule(void);

/**
 * @brief Change current stack pointer from MSP to PSP of current task.
 *        Requires "uint32_t *get_psp_of_current_task(void);" function
 */
__attribute((naked)) void change_sp_to_psp(void);

/**
 * @brief Init SysTick timer and enable the interrupt
 */
void initial_systick_config(void);

/**
 * @brief     Put initial scheduler stack value to MSP (Main stack pointer)
 * @param[in] start_of_stack - starting address of memory region allocated for scheduler stack.
 *            (Actually max used address, because stack will grow in descending order).
 */
__attribute((naked)) void init_scheduler_stack(void *start_of_stack);

/**
 * @brief     Put initial values of context registers into stack, to be used by scheduler when task is
 *            switched to active on the CPU.
 * @param[in] task_descriptor - Task Control Block
 */
void init_task_stack(TCB_t *task_descriptor);

#endif /* HAL_AND_ISRS_H_ */
