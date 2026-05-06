/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.h - LED PWM 子类
 *
 * 和 led_gpio 一样嵌套 struct led_base 在第一个位置, 但 PWM 子类
 * 自己的硬件字段是 channel + duty, 不是 pin。
 *
 * 同一个 led_base, 多种子类共用, 各自只放自己的硬件参数。
 */

#ifndef LED_PWM_H
#define LED_PWM_H

#include "led_base.h"

struct led_pwm {
	struct led_base base;   /* 公共部分 - 必须放第一个 */
	uint8_t         channel;
	uint8_t         duty;
};

int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty);
int led_pwm_on(struct led_pwm *me);
int led_pwm_off(struct led_pwm *me);

#endif /* LED_PWM_H */
