/* SPDX-License-Identifier: MIT */
/**
 * @file  led.h
 * @brief 父类统一接口 led_on / led_off / led_toggle
 *
 * @details
 * 上一章 ch10 应用层还得直接写 me->ops->on(me) 三行才能开/关/翻转.
 * 这一章把这三行包装成 led_on / led_off / led_toggle, 写在父类
 * (led_base.c) 里, 所有子类共用. 应用层只调 led_on(base), 不再
 * 看到 ops 字段, 也不需要知道这颗 LED 是 GPIO 还是 PWM.
 */

#ifndef LED_H
#define LED_H

#include "led_base.h"

typedef int (*led_action_fn)(struct led_base *me);

struct led_ops {
	led_action_fn on;
	led_action_fn off;
	led_action_fn toggle;
};

/* GPIO 子类 */
struct led_gpio {
	struct led_base base;
	uint8_t         pin;
};

/* PWM 子类 */
struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin);
int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty);

/*
 * 父类统一接口 - 写在 led_base.c, 所有子类共用.
 * 应用层只调 led_on / led_off / led_toggle, 看不到 ops 字段.
 */
int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_toggle(struct led_base *me);

extern const struct led_ops led_ops_gpio;
extern const struct led_ops led_ops_pwm;

#endif /* LED_H */
