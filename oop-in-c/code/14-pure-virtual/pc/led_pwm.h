/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.h - PWM LED 子类 (ch14 版, 三件套全填)
 *
 * PWM 子类三件套全填 (on / off / set_brightness), 演示 ch14 § 14.3
 * 的"选填字段填了就走子类"分支. 跟 GPIO 子类合在一起看到的是: 同一
 * 张 ops 表字段集, 子类各自决定填哪几项, 父类统一接口里负责处理
 * NULL.
 */

#ifndef LED_PWM_H
#define LED_PWM_H

#include "led.h"

/* PWM 子类: 三件套全填 */
struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};

int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty);


#endif /* LED_PWM_H */
