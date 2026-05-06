/* SPDX-License-Identifier: MIT */
/*
 * container_of.h - 与 ch13 一字不变（最小可用版）
 *
 * ch14 所有子类实现函数 (gpio_on / pwm_on / ...) 第一行依然用
 * container_of(me, struct led_xxx, base) 反推子类对象指针。
 * 完整解释见 ch13 § 13.4 + 13.7。
 */
#ifndef MY_CONTAINER_OF_H
#define MY_CONTAINER_OF_H

#include <stddef.h>

#define container_of(ptr, type, member)					\
	((type *)((char *)(ptr) - offsetof(type, member)))

#endif
