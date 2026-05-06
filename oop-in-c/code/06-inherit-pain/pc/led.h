/* SPDX-License-Identifier: MIT */
/*
 * led.h - LED GPIO 子类
 *
 * 见书 ch06 § 6.3 提公因式 + § 6.7.1 为什么 base 必须放第一个。
 *
 * struct led_gpio 第一个字段是 struct led_base, 后面才是 GPIO LED
 * 自己的硬件字段 (pin)。
 *
 * 注意: 第一个字段必须是 base。这条铁律不只是约定 ——
 * C11 标准 6.7.2.1 节保证 struct 第一个成员的偏移量是 0，
 * 这意味着 &gpio->base 的地址 == gpio 本身的地址，向上转型
 * (struct led_base *)&red_led 不会读到错位的字段。第 12 章
 * 会展开这条性质让"应用层只见基类指针"成为可能。
 *
 * Linux 内核 / Zephyr / GObject 三个全球最大的 OOP-in-C 项目
 * 都把"基类放第一个"列为硬规则，几十亿行 C 代码验证过。
 */

#ifndef LED_H
#define LED_H

#include "led_base.h"

struct led_gpio {
	struct led_base base;   /* 公共部分 - 必须放第一个 */
	uint8_t         pin;    /* GPIO 引脚号 - GPIO 子类特有 */
};

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin);
int led_gpio_on(struct led_gpio *me);
int led_gpio_off(struct led_gpio *me);

#endif /* LED_H */
