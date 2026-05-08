/* SPDX-License-Identifier: MIT */
/**
 * @file  led_pwm.h
 * @brief LED PWM 子类
 */

#ifndef LED_PWM_H
#define LED_PWM_H

#include "led_base.h"

struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};

int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty);


#endif /* LED_PWM_H */
