/* SPDX-License-Identifier: MIT */
/**
 * @file  led_pwm.h
 * @brief LED PWM 子类
 *
 * @details
 * 和 led_gpio 一样嵌套 struct led_base 在第一个位置, 但 PWM 子类
 * 自己的硬件字段是 channel + duty, 不是 pin.
 *
 * 一个 led_pwm 实例配一张 led_ops_pwm 表, 跟 led_ops_gpio 各跑各的.
 * 同一个 test_led 收到不同 ops, 跑出不同行为.
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

extern const struct led_ops led_ops_pwm;

#endif /* LED_PWM_H */
