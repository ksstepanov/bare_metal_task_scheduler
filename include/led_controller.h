/*
 * led_controller.h
 *
 *  Created on: May 16, 2025
 *      Author: konstantin
 */

#ifndef LED_CONTROLLER_H_
#define LED_CONTROLLER_H_
#include "common.h"

typedef enum led_state_ {
	LED_ON,
	LED_OFF
} led_state_t;

typedef enum led_ {
	LED_GREEN,
	LED_ORANGE,
	LED_RED,
	LED_BLUE
} led_t;

void turn_led(led_t led, led_state_t on_off);

void init_leds(void);

#endif /* LED_CONTROLLER_H_ */
