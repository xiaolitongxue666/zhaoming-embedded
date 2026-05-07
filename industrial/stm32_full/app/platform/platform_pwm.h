/* SPDX-License-Identifier: MIT */
/*
 * platform_pwm.h - PWM device driver framework (简化版).
 *
 * 提供给 led_pwm 这种调亮度子类用. 字符串名 / 通道号都是平台层细节,
 * driver 层只看到 channel + duty 抽象, 跨芯片移植时只换 pwm_board.c.
 *
 * duty 取值范围: 0-255 (8 位线性, 与 led_set_brightness 语义一致).
 */

#ifndef PLATFORM_API_PLATFORM_PWM_H_
#define PLATFORM_API_PLATFORM_PWM_H_

#include <stdint.h>

#include "platform/platform_def.h"

/* PWM duty 范围: 0-255 (线性, 8 位) */
#define PLATFORM_PWM_DUTY_MIN         0
#define PLATFORM_PWM_DUTY_MAX         255

/* ops 表抽象 (子类填写) */
struct platform_pwm_ops {
	platform_err_t (*enable)(int32_t channel);
	platform_err_t (*disable)(int32_t channel);
	platform_err_t (*set_duty)(int32_t channel, uint8_t duty);
};

/* 注册接口 (子类用) */
platform_err_t platform_pwm_register(const struct platform_pwm_ops *ops);

/* 公共 API (上层调) */
platform_err_t platform_pwm_enable(int32_t channel);
platform_err_t platform_pwm_disable(int32_t channel);
platform_err_t platform_pwm_set_duty(int32_t channel, uint8_t duty);

#endif /* PLATFORM_API_PLATFORM_PWM_H_ */
