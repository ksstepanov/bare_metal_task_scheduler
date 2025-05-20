/*
 * task.c
 *
 *  Created on: May 14, 2025
 *      Author: konstantin
 */
#include "led_controller.h"
#include "scheduler.h"

/**
 * @brief Idle task runs when all other taks are in TASK_BLOCKED state
 */
void task_idle(void)
{
	while(1);
}

/**
 * @brief User task handler. Can be populated with anything.
 *	  Current example turns on and off a led with specific period. 
 */
void task_1_handler(void)
{
	while(1) {
	#if (defined(DEBUG_ON) && defined(OPENOCD_SEMIHOSTING_ENABLED))
		printf("%s\n", __func__);
	#endif
		turn_led(LED_GREEN, LED_ON);
		delay_task(DELAY_1S);
		turn_led(LED_GREEN, LED_OFF);
		delay_task(DELAY_1S);
	}
}

/**
 * @brief User task handler. Can be populated with anything.
 *	  Current example turns on and off a led with specific period. 
 */
void task_2_handler(void)
{
	while(1) {
	#if (defined(DEBUG_ON) && defined(OPENOCD_SEMIHOSTING_ENABLED))
		printf("%s\n", __func__);
	#endif
		turn_led(LED_ORANGE, LED_ON);
		delay_task(DELAY_2S);
		turn_led(LED_ORANGE, LED_OFF);
		delay_task(DELAY_2S);
	}
}

/**
 * @brief User task handler. Can be populated with anything.
 *	  Current example turns on and off a led with specific period. 
 */
void task_3_handler(void)
{
	while(1) {
	#if (defined(DEBUG_ON) && defined(OPENOCD_SEMIHOSTING_ENABLED))
		printf("%s\n", __func__);
	#endif
		turn_led(LED_RED, LED_ON);
		delay_task(DELAY_4S);
		turn_led(LED_RED, LED_OFF);
		delay_task(DELAY_4S);
	}
}

/**
 * @brief User task handler. Can be populated with anything.
 *	  Current example turns on and off a led with specific period. 
 */
void task_4_handler(void)
{
	while(1) {
	#if (defined(DEBUG_ON) && defined(OPENOCD_SEMIHOSTING_ENABLED))
		printf("%s\n", __func__);
	#endif
		turn_led(LED_BLUE, LED_ON);
		delay_task(DELAY_8S);
		turn_led(LED_BLUE, LED_OFF);
		delay_task(DELAY_8S);
	}
}

