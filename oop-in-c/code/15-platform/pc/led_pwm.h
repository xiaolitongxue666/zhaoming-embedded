/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.h - 子类二: PWM LED (ch15 完整版, 风格 A)
 *
 * 通过 PWM 占空比驱动. 硬件资源: 一路 PWM 通道 + 当前占空比.
 * 三件套全填, 支持调光 (set_brightness 实填).
 *
 * 这一份子类头只装 struct led_pwm + 构造函数声明. 应用层永远不该
 * #include 它 -- 应用层只 #include "leds.h" 拿 base 句柄.
 */

#ifndef LED_PWM_H
#define LED_PWM_H

#include "led_base.h"

struct led_pwm {
	struct led_base base;       /* 父类, 第 0 字段 */
	uint8_t         channel;
	uint8_t         duty;
};

int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty);

#endif /* LED_PWM_H */
