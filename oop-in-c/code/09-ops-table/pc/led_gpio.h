/* SPDX-License-Identifier: MIT */
/**
 * @file  led_gpio.h
 * @brief LED GPIO 子类
 *
 * @details
 * 本章 ch09 故意保持 ch07/ch08 的子类字段集 (base + 硬件参数), 不在
 * 子类里挂 ops 字段. ops 字段进 base 是 ch10 的事.
 *
 * 一个 led_gpio 实例配一张 led_ops_gpio 表. 表本身在 .rodata 段,
 * 全程序共享, 100 颗 GPIO LED 共享同一张 12 字节的表.
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

/*
 * led_ops_gpio - GPIO 风格的 ops 表 (extern, 定义在 led_gpio.c).
 *
 * const + extern 让这张表落进 .rodata 段 -- MCU 上烧到 Flash, 全程序
 * 共享, 链接期不可改, 防止运行时被改成野指针 (见 ch09 § 9.5.3).
 */
extern const struct led_ops led_ops_gpio;

#endif /* LED_GPIO_H */
