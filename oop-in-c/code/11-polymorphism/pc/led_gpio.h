/* SPDX-License-Identifier: MIT */
/**
 * @file  led_gpio.h
 * @brief LED GPIO 子类
 *
 * @details
 * 子类字段集跟 ch10 一字不变. 本章 ch11 的演化点是: 父类暴露
 * led_on / led_off / led_toggle 统一接口, 应用层不再直接调
 * led_gpio_xxx, 也不再走 me->ops->on(me) 长串. 这份子类 .h
 * 只装 struct led_gpio + 构造函数. ops 表 static 收回
 * led_gpio.c 内部 -- 子类 init 第一行 led_base_init(&me->base,
 * name, &gpio_ops) 已经把 ops 表交给 base, 外面拿不到也用不到.
 */

#ifndef LED_GPIO_H
#define LED_GPIO_H

#include "led_base.h"

struct led_gpio {
	struct led_base base;
	uint8_t         pin;
};

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin);


#endif /* LED_GPIO_H */
