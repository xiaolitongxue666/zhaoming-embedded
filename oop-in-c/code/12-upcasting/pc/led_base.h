/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief led_base 字段集 + 通用 init - 跨子类共享一份初始化代码
 *
 * @details
 * 字段集跟 ch10 / ch11 一字不变 (ops + name + is_on). 父类层提供
 * led_base_init, 三种子类 (GPIO / PWM / I2C) 的 init 第一行都调它,
 * 把 "我用哪张 ops 表" 当常量参数传进来.
 *
 * 这一层抽象的好处: 哪天 base 加了一个公共字段 (例如 owner / lock),
 * 只改 led_base_init 这一个点, 三种子类的 init 不用动.
 *
 * ch12 主题是向上转型, 主线告诉你 &gpio_led.base 这个指针怎么扮演
 * struct led_base * 句柄. 父类 init 这一层不是本章新引入的设计,
 * 是从 ch10 一路继承下来的形态.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include <stdint.h>
#include <stdbool.h>

struct led_ops;

/*
 * struct led_base - 父类.
 *
 * 三个字段:
 *   ops    : 子类的操作表入口, 必须放第一个 (向上转型零开销)
 *   name   : 给日志打印, 也是子类的 "我是谁" 标识
 *   is_on  : 当前开关状态
 */
struct led_base {
	const struct led_ops *ops;     /* 第一个字段, 对象起始地址处 */
	const char           *name;
	bool                  is_on;
};

int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);

#endif /* LED_BASE_H */
