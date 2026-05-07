/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief led_base 字段集 + 共有 init (ch14 版, 沿用 ch10/ch13 一字不变)
 *
 * @details
 * 字段集和共有 init 接口跟 ch10/ch11/ch13 一字不变. 这一章所有的变化
 * 都在 led.c 里 (父类统一接口里加三种 NULL 处理策略), led_base 自身
 * 不动:
 *
 *     struct led_base {
 *         const struct led_ops *ops;     // 第 0 字段
 *         const char           *name;
 *         bool                  is_on;
 *     };
 *
 * 子类 init (led_gpio_init / led_pwm_init) 第一行调 led_base_init, 把
 * "我用哪张 ops 表"作为常量参数传进来, 一次填好.
 *
 * 见 ch14 § 14.2 (必填) / § 14.3 (选填).
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

/*
 * 前向声明 - led_ops 完整定义在 led.h. led_base.h 只用到 const struct
 * led_ops * 指针类型, 不依赖字段集. 减少头文件耦合.
 */
struct led_ops;

struct led_base {
	const struct led_ops *ops;     /* 第 0 字段 */
	const char           *name;
	bool                  is_on;
};

/*
 * led_base_init - 共有字段 init.
 *
 * 子类 init 第一行调这个函数, 把对应的 const ops 表作为常量传进来.
 * 共有字段 (ops / name / is_on) 一次填好.
 */
int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);

#endif /* LED_BASE_H */
