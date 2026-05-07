/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief LED 基类 + 操作表 (struct led_ops) + test_led 测试入口
 *
 * @details
 * 沿用 ch07/ch08 的 base 字段集 (name + is_on, ops 字段在 ch10 才进 base).
 * 本章新增的是 struct led_ops -- 把多个函数指针打包成一张操作接口表.
 * struct led_ops 放这里, 让所有 LED 子类 (gpio/pwm) include 一份就够,
 * 不用各自重复声明.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

struct led_base {
	const char *name;
	bool        is_on;
};

int led_base_init(struct led_base *me, const char *name);
const char *led_base_get_name(const struct led_base *me);

/*
 * led_action_fn - 用 typedef 给函数指针类型起短名.
 *
 * "int (*)(struct led_base *)" 写一次还行, ops 表里写三次就累.
 * Linus 反对 typedef struct (藏类型信息), 但函数指针 typedef 是
 * 他公开支持的例外: 类型字面量太长, 起短名纯收益.
 * 见 ch09 § 9.2.
 */
typedef int (*led_action_fn)(struct led_base *me);

/*
 * struct led_ops - 操作表.
 *
 * 把同一种 LED 的所有"可变行为"打包. 一种 LED 实现填一张表,
 * test_led 内部按名字 ops->on / ops->off 访问, 再也不会传反.
 *
 * 按名访问还有一个好处: 加新行为 (set_brightness) 只在 ops 里加
 * 字段, 老的 on/off 调用一字不改 (designated initializer 把
 * 没列出的字段默认填 NULL, 见 ch09 § 9.5.1).
 */
struct led_ops {
	led_action_fn on;
	led_action_fn off;
	led_action_fn toggle;
};

/*
 * test_led - 接 ops 指针, 一个参数搞定三件事 (on/off/toggle).
 *
 * 第二个参数声明成 const struct led_ops * 而不是 struct led_ops *,
 * 表示 test_led 不会改 ops 表内容.
 */
int test_led(struct led_base *me, const struct led_ops *ops);

#endif /* LED_BASE_H */
