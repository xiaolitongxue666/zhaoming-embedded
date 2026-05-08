/* SPDX-License-Identifier: MIT */
/**
 * @file  led_pwm.h
 * @brief LED PWM 子类
 *
 * @details
 * 和 led_gpio 一样: base + 硬件参数 (channel + duty).
 * led_pwm_init 把 &led_ops_pwm 交给 led_base_init.
 */

#ifndef LED_PWM_H
#define LED_PWM_H

#include "led_base.h"

/* PWM 子类: channel + duty 在子类里 */
struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};

int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty);


#endif /* LED_PWM_H */
