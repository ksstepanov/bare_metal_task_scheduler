/*
 * hal_and_isrs.c
 *
 *  Created on: May 14, 2025
 *      Author: konstantin
 */

#include "common.h"
#include "scheduler.h"
#include "hal_and_isrs.h"

void printf_func(const char *func) {
	printf("%s\n", func);
}

struct USFR_err_info {
	uint8_t code;
	const char *msg;
};

static const struct USFR_err_info USFR_err_info_map[] = {
		{0, "INV instruction"},
		{1, "INV processor state"},
		{2, "INV PC load by EXC_RETURN"},
		{3, "No coprocessor UsageFault"},
		{8, "Unaligned access"},
		{9, "Div by 0"}
};

static const uint8_t N_USFR_err_info_codes = ARRAY_SIZE(USFR_err_info_map);

static const char* stack_dump_msgs[] = {
		"XPSR: ",
		"PC:   ",
		"LR:   ",
		"R12:  ",
		"R3:   ",
		"R2:   ",
		"R1:   ",
		"R0:   "
};

/**
 * @brief     Auxiliary function for UsageFault system exception handler. Dumps register content.
 * @param[in] pBaseStackFrame - main stack pointer value (MSP) captured just after context saving.
 *                              Contains (R0, R1, R2, R3, LR, PC, xPSR)
 */
static void dump_stack_frame(uint32_t *pBaseStackFrame) {
	/*
	 * XPSR
	 * PC // gives instruction that resulted to fault
	 * LR // gives return address of the instruction which should be next in normal case
	 * R12
	 * R3
	 * R2
	 * R1
	 * R0 //<-MSP
	 */
	const uint8_t n_items = ARRAY_SIZE(stack_dump_msgs);
	uint8_t offset = 0;
	for (int i = 0; i < n_items; i++) { // 0 .. 7
		offset = ((n_items - 1) - i);   // 7 .. 0
		uint32_t val = pBaseStackFrame[offset];
		printf("\t%s %p: %lx\n", stack_dump_msgs[i], &pBaseStackFrame[offset], val);
	}
}

/**
 * @brief     Auxiliary function implements UsageFault system exception handler.
 * @param[in] pBaseStackFrame - main stack pointer value (MSP) captured just after context saving.
 *                              Contains (R0, R1, R2, R3, LR, PC, xPSR)
 */
static void UsageFault_Handler_c(uint32_t *pBaseStackFrame) {
	/* Next will not work because of the function epilogue sequence, stack is already modified */
	// __asm volatile ("MRS r0, MSP");
	// register uint32_t msp_val __asm ("r0"); // map r0 to a variable msp_val

	printf_func(__func__);

	volatile uint32_t *status = (void *)(SCB_USFR);
	uint16_t status_val = (*status) & (0xFFFF);
	printf("Exception: Usage FAULT\n");
	printf("Status: %x\n", status_val); // only 16 bits LBS are used
	int known_status = 0;
	for (int i = 0; i < N_USFR_err_info_codes; i++ ) {
		if ((1 << USFR_err_info_map[i].code) == status_val) {
			known_status = 1;
			printf("%s\n", USFR_err_info_map[i].msg);
			break;
		}
	}
	if (known_status == 0) {
		printf("Unknown error\n");
	}
	dump_stack_frame(pBaseStackFrame);
	while(1);
}

/* =============== Scheduler calls implementation: =================== */
/**
 * @brief Enable all System Fault handlers (Usage, Memory, Bus)
 */
void enable_all_configurable_exceptions(void) {
	volatile uint32_t *p_SCB_SHCSR = (void *)(SCB_SHCSR);
	*p_SCB_SHCSR |= ((1 << SCB_USAGE_FAULT_EN_BIT) | (1 << SCB_BUS_FAULT_EN_BIT | (1 << SCB_BUS_FAULT_EN_BIT)));
}

/**
 * @brief Force trigger scheduler (PendSV Handler for context switching)
 */
void schedule(void)
{
	// Set PendSV handler bit:
	volatile uint32_t *pICSR = (void *)(SCB_ICSR);
	*pICSR |= (1 << SCB_ICSR_PEND_SV_EN_BIT);
}

/**
 * @brief Change current stack pointer from MSP to PSP of current task.
 *        Requires "uint32_t *get_psp_of_current_task(void);" function
 */
__attribute((naked)) void change_sp_to_psp(void) {  // Nacked function doesn't have epilogue and prologue sequences
													// (doesn't save/restore any context)
	// 1. Initialize PSP with initial stack value:
	__asm volatile ("PUSH {LR}"); 					// preserve LR that connects to init_scheduler
	__asm volatile ("BL get_psp_of_current_task"); 	// Branch and link because we need to come back to this function. BUT this modifies LR!
	__asm volatile ("MSR PSP, R0");					// R0 holds return value, copy it to PSP
	__asm volatile ("POP {LR}");

	// Set CONTROL register SLSEL bit[1] to 1
	__asm volatile ("MRS R0,CONTROL");
	__asm volatile ("ORR R0, R0, #0x2");
	__asm volatile ("MSR CONTROL, R0");
	__asm volatile ("BX LR");
}

/**
 * @brief Init SysTick timer and enable the interrupt
 */
void initial_systick_config(void)
{
	// Configure task duration: RVR holds reset value which is decremented each processor tick
	volatile uint32_t *pResetVal = (void *)(SYSTICK_RVR);
	*pResetVal &= ~(0x00FFFFFF); // clear last 24 bits
	*pResetVal |= SYSTICK_RESET_VAL;

	// Enable timer and interrupt. Interrupt will be triggered on every tick where timer val overflows (0->RVR)
	volatile uint32_t *pControl = (void *)(SYSTICK_CSR);
	*pControl |= ((1 << SYSTICK_CSR_ENABLE_BIT) | (1 << SYSTICK_CSR_ENABLE_INTERRUPT_BIT) | (1 << SYSTICK_CSR_CLKSOURCE_BIT));
}

/**
 * @brief     Put initial scheduler stack value to MSP (Main stack pointer)
 * @param[in] start_of_stack - starting address of memory region allocated for scheduler stack.
 *            (Actually max used address, because stack will grow in descending order).
 */
__attribute((naked)) void init_scheduler_stack(void *start_of_stack)
{
	__asm volatile ("MSR MSP, R0"); // because R0 holds the first argument
	__asm volatile ("BX LR");
}

/**
 * @brief     Put initial values of context registers into stack, to be used by scheduler when task is
 *            switched to active on the CPU.
 * @param[in] task_descriptor - Task Control Block
 */
void init_task_stack(TCB_t *task_descriptor)
{
	// There are 16 total registers to be initialized:
	uint32_t *init_stack_frame_end = task_descriptor->stack_start - 16;
	// Init all general purpose registers:
	for (int i = 0; i < CONTEXT_GP_REGS; i++) {
		init_stack_frame_end[i] = TINIT_GEN_PURP_REG_VAL;
	}
	// init LR, PC and PSR:
	init_stack_frame_end[CONTEXT_GP_REGS] = TINIT_LR_VAL;
	init_stack_frame_end[CONTEXT_GP_REGS + 1] = (uint32_t)(void *)task_descriptor->handler;
	init_stack_frame_end[CONTEXT_GP_REGS + 2] = TINIT_PSR_VAL;

	// Finally save the PSP (new top of the stack) to global structure:
	task_descriptor->stack_start = init_stack_frame_end;
}


/* ================================== ISRS and exception handlers =========================== */
/**
 * @brief Fault handler in case of:
 *        - execution of undefined instruction
 *        - use FPU when it is disabled
 *        - trying to return to Thread mode when interrupt is still active
 *        - unaligned memory access
 *        - div by 0
 */
__attribute((naked)) void UsageFault_Handler(void) {
	// save the SP just after exception happened:
	__asm volatile ("MRS r0, MSP"); // R0 will hold SP address and it will be the first argument of the next function:
	__asm volatile ("B UsageFault_Handler_c"); // branch to
}

/**
 * @brief Context switch handler. Triggered by SysTick exception or manually from the task when it is delayed
 */
__attribute((naked)) void PendSV_Handler(void)
{
	// 1. Get the context of current task and save it to its stack:
	//     1.1 Get current task's PSP
	//     1.2 Save R4-R11 to stack
	//     1.3 Save current PSP to global var tasks
	__asm volatile ("MRS R0, PSP");

	/* Next works, but can be simplified with STMDB */
	/*__asm volatile ("SUBS R0, #64"); // reserve space for 8 values
	__asm volatile ("STR R4,[R0, #0]");
	...
	__asm volatile ("STR R11,[R0, #28]");*/
	__asm volatile ("STMDB R0!,{R4-R11}");  // DB = Decrement Before and then store.
											// ! means update the register after decrement
	__asm volatile ("PUSH {LR}");
	__asm volatile ("BL save_psp_value");

	// 2. Decide on next task to run and restore it's context
	// 2.1 Find psp of next task
	// 2.2 pop R4-R11 and set correct PSP
	//
	__asm volatile ("BL update_to_next_task"); // Increment global variable current task
	__asm volatile ("BL get_psp_of_current_task"); // put PSP from global var to R0
	__asm volatile ("LDMIA R0!,{R4-R11}"); // TODO! Check if not R11-R4
	__asm volatile ("MSR PSP, R0");
	__asm volatile ("POP {LR}");
	__asm volatile ("BX LR");
}

/**
 * @brief Triggered by SysTick timer every TASK_DURATION interval. Implements scheduler tick.
 */
void SysTick_Handler(void)
{
	update_global_tick_count();
	update_blocked_tasks();

	// Set PendSV handler bit:
	schedule();
}

/**
 * @brief Fault handler with priority -1 in case of:
 *        - escalation or error during lower priority exception processing
 *        - exception that can't be managed by higher priority exceptions (f. ex not implemented handler or
 *          bus error when fetching vector table)
 *        - execute SVC instruction during SVC handler
 */
void HardFault_Handler(void) {
	printf_func(__func__);
	while(1);
}

/**
 * @brief Fault handler in case of:
 *        - memory access violation(MPU protected, execute code from XN region, unpriviledged thread mode code
 *          accesses protected registers)
 */
void MemManage_Handler(void) {
	printf_func(__func__);
	while(1);
}

/**
 * @brief Fault handler in case of:
 *        - error responce returned by the processor bus interfaces (for example access invalid memory address or
 *          bus device is not ready)
 */
void BusFault_Handler(void) {
	printf(__func__);
	while(1);
}
