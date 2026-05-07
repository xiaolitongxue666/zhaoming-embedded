/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief led_base 字段集 + 共有 init 接口 (ch13 版)
 *
 * @details
 * 字段集跟 ch10/ch11 一字不变. ops 仍然是 led_base 的第一个字段,
 * 这一条是 led_base 自身的 ABI 不变量, 不被 ch13 的"教学变形"影响:
 *
 *     struct led_base {
 *         const struct led_ops *ops;     // 第 0 字段 (base 自身视角)
 *         const char           *name;
 *         bool                  is_on;
 *     };
 *
 * 注意一个区别:
 *   - "ops 在 led_base 的第 0 字段" -- 永远成立 (base 自身视角)
 *   - "led_base 在 led_gpio 的第 0 字段" -- ch13 故意打破 (容器视角)
 * 第二条原本撑住了 ch12 那一招 (struct led_gpio *)me 强转, 一旦
 * 容器里 base 不在第 0 偏移, 强转就会算错地址. 这正是 ch13 要解的
 * 问题, 也是 container_of 的核心威力.
 *
 * 见 ch13 § 13.3 强转能用但脆弱 + § 13.6 在 gpio_on 里用一下.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

/*
 * 前向声明 - led_ops 完整定义在 led.h.
 * led_base.h 只用到 const struct led_ops * 指针类型, 不依赖字段集,
 * 减少头文件耦合. ch14 给 led_ops 加新字段时, 不会触发 led_base.h
 * 重新编译.
 */
struct led_ops;

struct led_base {
	const struct led_ops *ops;     /* 第 0 字段 (base 自身视角) */
	const char           *name;
	bool                  is_on;
};

/*
 * led_base_init - 共有字段 init.
 *
 * 子类 init (led_gpio_init / led_pwm_init / led_i2c_init) 第一行调
 * led_base_init, 把"我用哪张 ops 表"作为常量传进来. 一次填好,
 * 之后对象不再碰 ops / name / is_on.
 */
int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);

#endif /* LED_BASE_H */
