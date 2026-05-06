/* SPDX-License-Identifier: MIT */
/**
 * @file  led.h
 * @brief 子类 + ops 表 + 基类层多态接口
 *
 * @details
 * 本章 base 已经带 ops 字段了 (见 led_base.h), 子类不再需要自己挂
 * 函数指针. 应用层调用形态变成:
 *   led_gpio_init(&red_led, "red", 13);    // 子类 init 把 &led_ops_gpio
 *                                          //   作为常量传给 base init
 *   led_on(&red_led.base);                 // 应用层只看见 base 指针
 *
 * led_on/off/toggle 三个对外 API 接 struct led_base *, 函数体一行胶水:
 *   return me->ops->on(me);
 * 应用层完全不知道下面挂的是哪种子类, 这就是多态 (ch11 完整展开).
 *
 * 注: ch10 这一份代码里 led_gpio / led_pwm 子类只有 base + 硬件参数,
 * "led_ops 字段不放进子类"的决策见 ch10 § 10.8.5.
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

struct led_gpio {
	struct led_base base;
	uint8_t         pin;
};

struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin);
int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty);

/*
 * 应用层调 led_on(&red_led.base) 就行. 函数体是
 *     return me->ops->on(me);
 * 不用应用层传 ops.
 */
int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_toggle(struct led_base *me);

extern const struct led_ops led_ops_gpio;
extern const struct led_ops led_ops_pwm;

#endif /* LED_H */
