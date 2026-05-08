/* SPDX-License-Identifier: MIT */
/**
 * @file  led_gpio.h
 * @brief LED GPIO 子类 (ch12 版)
 *
 * @details
 * 子类 .h 只装两样东西: struct led_gpio 字段集 (base 在第 0 字段) +
 * 构造函数 led_gpio_init 声明. ops 表 (gpio_on / gpio_off + gpio_ops
 * 表) static 锁在 led_gpio.c 内, 应用层永远碰不到 gpio_xxx 这一层.
 *
 * 见 ch12 § 12.4 板级初始化 + § 12.6 换硬件方案三行改动.
 */

#ifndef LED_GPIO_H
#define LED_GPIO_H

#include "led_base.h"

/*
 * GPIO LED 子类: 一个 GPIO 引脚拉高 (或拉低) 点亮.
 *
 * base 必须在第一个字段, 后面跟硬件资源. on_level 让同一份 led_gpio
 * 子类支持两种接法 (高电平点亮 = LED 阴极接 GPIO / 低电平点亮 = LED
 * 共阳极反接). 硬件极性差异封装在子类里, 应用层 led_on(handle) 不用
 * 知道接的是高还是低.
 */
struct led_gpio {
	struct led_base base;       /* 父类, 第 0 字段 (向上转型不变量) */
	uint8_t         pin;
	bool            on_level;   /* 1 = 高电平点亮, 0 = 低电平点亮 */
};

int led_gpio_init(struct led_gpio *me, const char *name,
                  uint8_t pin, bool on_level);


#endif /* LED_GPIO_H */
