/* SPDX-License-Identifier: MIT */
/*
 * container_of.h - 与 ch13 一字不变 (最小可用版)
 *
 * ch15 三个子类 .c (led_gpio.c / led_pwm.c / led_i2c.c) 的实现函数
 * (gpio_on / pwm_on / i2c_on) 第一行依然用 container_of 反推子类对象.
 * 完整解释见 ch13 § 13.5 三步宏 container_of.
 */
#ifndef MY_CONTAINER_OF_H
#define MY_CONTAINER_OF_H

#include <stddef.h>

#define container_of(ptr, type, member)					\
	((type *)((char *)(ptr) - offsetof(type, member)))

#endif
