/*
 * led_controller.c
 *
 *  Created on: May 16, 2025
 *      Author: konstantin
 */


#include "led_controller.h"
// LED GPIO COLOR
// 1   PE0  Green
// 2   PE1  Orange
// 3   PE2  Red
// 4   PE3  BLUE

// 0 turns LED on

#define AHB1_BASE (0x40020000U)

// 1. RCC:
/* Reset and clock control */
#define RCC_BASE (AHB1_BASE + 0x3800)
#define RCC ((RCC_RegDef_t *)RCC_BASE)

#define GPIOX_OFFSET (0x400U)
#define GPIOE_BASE (AHB1_BASE + 4 * GPIOX_OFFSET)
#define GPIOE ((GPIO_RegDef_t *)GPIOE_BASE)
#define GPIOE_PCLK_EN() (RCC->AHB1ENR |= (1 << 4))

static volatile uint32_t *pLedCtrl = (void *)(GPIOE_BASE + 0x14);

void init_leds(void)
{
	// Enable clock to GPIOE:
	volatile uint32_t *pRCC_AHB1_EN = (void *)(RCC_BASE + 0x30);
	*pRCC_AHB1_EN |= (1 << 4);

	// Enable 4 ports of GPIOE: PE0 - PE3 to output:
	volatile uint32_t *pGPIOE_MODE = (void *)(GPIOE_BASE);
	*pGPIOE_MODE &= ~(0xFF); // clear 8 LBSs
	*pGPIOE_MODE |= 0x55; // put output_mode = "01" to last 8 bits, resulting to 01010101

	// Make all leds off first:
	*pLedCtrl |= 0xF;
}

void turn_led(led_t led, led_state_t on_off) {
	if (on_off == LED_OFF)
		*pLedCtrl |= (1 << led);
	else
		*pLedCtrl &= ~(1 << led);
}


