/* SPDX-License-Identifier: MIT */
/**
 * @file  led_gpio.h
 * @brief LED GPIO 子类 -- ops 字段已在 led_base 里, 子类只装硬件参数
 *
 * @details
 * 上一章 ch09 的子类字段集 (base + 硬件参数) 这里一行不变. 区别在
 * led_gpio_init 内部: 把 &led_ops_gpio 作为常量传给 led_base_init,
 * base 把它存到 me->ops 字段, 一次填好.
 */

#ifndef LED_GPIO_H
#define LED_GPIO_H

#include "led_base.h"

/* GPIO 子类: pin 在子类里 */
struct led_gpio {
	struct led_base base;
	uint8_t         pin;
};

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin);

extern const struct led_ops led_ops_gpio;

#endif /* LED_GPIO_H */
