/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.h - PWM LED 子类 (ch13 版)
 *
 * PWM 子类 base 仍在第 0 字段, 但子类实现层 (led_pwm.c) 用 container_of
 * 反推自己, 跟 GPIO 子类风格保持一致. ch13 教学要求三种子类一视同仁
 * 全部用 container_of, 让 GPIO 那条 base 偏移 4 的奇葩布局看起来不
 * 突兀.
 *
 * PWM 子类填了 set_brightness, 按 duty 调亮度. ch14 主题展开"必填 +
 * 选填"机制, 这一章 (ch13) 主线是 container_of, 字段先挂上.
 */

#ifndef LED_PWM_H
#define LED_PWM_H

#include "led.h"

struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};

int led_pwm_init(struct led_pwm *me, const char *name,
		 uint8_t channel, uint8_t duty);

extern const struct led_ops led_ops_pwm;

#endif /* LED_PWM_H */
