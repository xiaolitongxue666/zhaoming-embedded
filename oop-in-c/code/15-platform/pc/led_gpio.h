/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.h - 子类一: GPIO LED (ch15 完整版, 风格 A)
 *
 * 最简单的 LED: 一个 GPIO 引脚拉高 (或拉低) 点亮.
 * base 必须在第一个字段 (零开销向上转型, 见 ch12 § 12.2).
 *
 * on_level 让同一份 led_gpio 子类支持两种接法: 高电平点亮 (LED 阴极
 * 接 GPIO, 阳极接 VCC) / 低电平点亮 (反过来, LED 共阳极). 不用为这
 * 两种接法各写一个 led_gpio 子类, bool 字段区分就行.
 *
 * 这一份子类头只装 struct led_gpio + 构造函数声明. 应用层永远不该
 * #include 它 -- 应用层只 #include "leds.h" 拿 base 句柄.
 */

#ifndef LED_GPIO_H
#define LED_GPIO_H

#include "led_base.h"

struct led_gpio {
	struct led_base base;       /* 父类, 第 0 字段 */
	uint8_t         pin;
	bool            on_level;   /* 1 = 高电平点亮, 0 = 低电平点亮 */
};

int led_gpio_init(struct led_gpio *me, const char *name,
                  uint8_t pin, bool on_level);

#endif /* LED_GPIO_H */
