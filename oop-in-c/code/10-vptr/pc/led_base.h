/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief led_base 加 ops 字段, 每颗 LED 自带操作表
 *
 * @details
 * 上一章的痛点：调用方每次都得自己传 ops 表
 *
 *     test_led(&red_led.base, &led_ops_gpio);
 *     test_led(&blue_led.base, &led_ops_pwm);
 *
 * 调用方得记住"红灯用 gpio 表、蓝灯用 pwm 表". 一旦传错,
 * 又是 bug.
 *
 * 这一章的解法：让每颗 LED 自己带着自己的 ops 表. struct led_base
 * 加一个 const struct led_ops *ops 字段, init 时填进去, 调用时直接
 * 从对象里取.
 *
 *     struct led_base {
 *         const struct led_ops *ops;     // 新增, 第一个字段
 *         const char *name;
 *         bool        is_on;
 *     };
 *
 * 把 ops 放在第一个字段是有原因的：
 *   1) &red_led 和 &red_led.base 是同一个地址
 *      (C99 § 6.7.2.1: "结构体第一个成员的地址等于结构体本身的
 *       地址"), 子类指针拿到时 0 偏移
 *   2) me->ops 取出来只用一条单字偏移指令, 不用先加再读
 *
 * 字段类型是 "const struct led_ops *" 而不是 "struct led_ops *
 * const":
 *   const 修饰 led_ops, 意思是"指向常量 led_ops 的指针". 表本身
 *   不允许改 (me->ops->on = ... 编译报错), 但指针可以重新指向另一
 *   张 ops 表. 工业代码 99% 用这种风格.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

/*
 * 前向声明 - led_ops 完整定义在 led.h.
 * 这里只用到指针类型, 编译器只需要知道"led_ops 是个 struct",
 * 不需要看到字段集. 这样 led_base.h 不依赖 led.h, 减少头文件
 * 耦合 -- 哪天 led_ops 字段改了, 不会触发 led_base.h 重新编译.
 */
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
