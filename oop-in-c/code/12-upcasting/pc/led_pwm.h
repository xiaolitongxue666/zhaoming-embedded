/* SPDX-License-Identifier: MIT */
/**
 * @file  led_pwm.h
 * @brief LED PWM 子类 (ch12 版)
 *
 * @details
 * 通过 PWM 占空比驱动 LED. 硬件资源: 一路 PWM 通道 + 当前占空比.
 * 子类 .h 只装字段集 + 构造函数 + ops 表 extern, 实现 (pwm_on /
 * pwm_off + pwm_ops) 锁在 led_pwm.c.
 */

#ifndef LED_PWM_H
#define LED_PWM_H

#include "led.h"

struct led_pwm {
	struct led_base base;       /* 父类, 第 0 字段 */
	uint8_t         channel;
	uint8_t         duty;
};

int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty);

extern const struct led_ops led_ops_pwm;

#endif /* LED_PWM_H */
