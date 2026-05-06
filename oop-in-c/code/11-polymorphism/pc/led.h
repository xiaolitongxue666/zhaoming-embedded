/* SPDX-License-Identifier: MIT */
/**
 * @file  led.h
 * @brief 多态完整版 - 应用层接口与 ch10 一致, 下游 platform 演化为 ops 表
 *
 * @details
 * led 子系统对外的 led_on / led_off / led_toggle 三个 API 跟 ch10
 * 字面量一致. 应用层一行不改也能跑.
 *
 * 但下游链路有大变化:
 *   ch10:  led_on -> me->ops->on(me) -> gpio_on() -> platform_gpio_write()
 *                                                    \_直接执行 GPIO 操作
 *   ch11:  led_on -> me->ops->on(me) -> gpio_on() -> platform_gpio_write()
 *                                                    \_内部走 g_platform_ops
 *                                                      ->gpio_write() dispatch
 * 也就是 platform 层这一层从单一实现变成多实现可切换, 但封装函数
 * 接口一字未动. 见 ch11 § 11.5 / platform_ops.h.
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

/* 应用层接口 - 接 base 指针, 内部 dispatch */
int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_toggle(struct led_base *me);

extern const struct led_ops led_ops_gpio;
extern const struct led_ops led_ops_pwm;

#endif /* LED_H */
