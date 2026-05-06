/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief 多态完整图景 - vptr + vtable + dispatch 三件齐
 *
 * @details
 * 走到本章, OOP 三大特性的"多态"那一块所有齿轮都已经齐了:
 *   1) vptr   - struct led_base 第一个字段 ops, 每个对象自带 (ch10)
 *   2) vtable - struct led_ops, 每种实现一张 const 表 (ch09)
 *   3) dispatch - led_on(base) 函数体 me->ops->on(me) (ch10/ch11)
 *
 * led_base 字段集与 ch10 完全一致, 教学版到本章定型 (ch11 § 11.12),
 * ch12 ~ ch18 不再变. 真正本章新东西在 platform 层: 从函数式
 * 封装演化成"对外封装 + 对内 ops 表" (见 platform_ops.h 和
 * platform_dispatch.c). 设备层和应用层一字不知, 但 platform 这层
 * 自己也变成了多态机制. 见 ch11 § 11.5.
 *
 * ch11 § 11.5 把这件事称作"OOP 的递归应用": 同一个 ops 机制,
 * 先用在设备层 (每颗 LED 自带 ops), 再用在平台层 (platform 层
 * 内部维护一个 ops 指针). 两层独立演化, 机制完全一致.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include <stdint.h>
#include <stdbool.h>

struct led_ops;

struct led_base {
	const struct led_ops *ops;     /* vptr - 必须是第一个字段 (见 ch10 § 10.3) */
	const char           *name;
	bool                  is_on;
};

int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);
const char *led_base_get_name(const struct led_base *me);

#endif /* LED_BASE_H */
