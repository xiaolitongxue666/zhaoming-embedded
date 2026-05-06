/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief LED 基类抽象 (ch07 与 ch06 一致, base 字段集不变)
 *
 * @details
 * 这一章把"调谁开灯"这件事从写死改成"运行时由字段决定", 演化点全在
 * 子类 struct led_gpio: 多了一个 on_func 函数指针字段. base 这一层
 * 没有引入新概念, 沿用 ch06 的 name + is_on. 见 ch07 § 7.3.
 *
 * ops 字段要等到 ch10 才放进 base. 本章先在子类里手动挂函数指针,
 * 让读者亲眼看见"加一个字段存函数地址"是什么意思, 再到 ch09/ch10
 * 把这件事系统化.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

struct led_base {
	const char *name;       /* 给日志打印用, 也是子类的"我是谁"标识 */
	bool        is_on;      /* 当前开关状态, 由 led_gpio_on/off 维护 */
};

int led_base_init(struct led_base *me, const char *name);
const char *led_base_get_name(const struct led_base *me);

#endif /* LED_BASE_H */
