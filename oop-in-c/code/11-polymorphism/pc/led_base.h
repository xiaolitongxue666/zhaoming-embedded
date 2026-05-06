/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief led_base 字段集 - ops 字段 + 共有数据
 *
 * @details
 * 字段集跟 ch10 一字不变. 本章 ch11 在父类 led_base 这一层加了
 * led_on / led_off / led_toggle 三个父类统一接口 (声明在 led.h,
 * 实现在 led_base.c), 让应用层只对 base 指针编程.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

struct led_ops;

struct led_base {
	const struct led_ops *ops;     /* 第一个字段, 对象起始地址处 */
	const char           *name;
	bool                  is_on;
};

int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);
const char *led_base_get_name(const struct led_base *me);

#endif /* LED_BASE_H */
