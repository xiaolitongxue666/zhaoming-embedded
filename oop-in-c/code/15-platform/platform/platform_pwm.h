/* SPDX-License-Identifier: MIT */
/*
 * platform_pwm.h - PWM 抽象层 (简化版).
 *
 * 提供给 led_pwm 这种调亮度子类用. duty 取值范围 0-255 (8 位线性).
 * 跨 MCU 移植只换 platform/arch/<mcu>/pwm_board.c (本章合并到 pin_board.c).
 */

#ifndef PLATFORM_PLATFORM_PWM_H
#define PLATFORM_PLATFORM_PWM_H

#include <stdint.h>

#define PLATFORM_PWM_DUTY_MIN    0
#define PLATFORM_PWM_DUTY_MAX    255

/* ops 表抽象 (子类填写) */
struct platform_pwm_ops {
	int (*enable)(int32_t channel);
	int (*disable)(int32_t channel);
	int (*set_duty)(int32_t channel, uint8_t duty);
};

/* 注册接口 (子类启动期用) */
int platform_pwm_register(const struct platform_pwm_ops *ops);

/* 公共 API */
int platform_pwm_enable(int32_t channel);
int platform_pwm_disable(int32_t channel);
int platform_pwm_set_duty(int32_t channel, uint8_t duty);

#endif /* PLATFORM_PLATFORM_PWM_H */
