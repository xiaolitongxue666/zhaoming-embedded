/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief LED 基类抽象 - 沿用 ch07/ch08 的 name + is_on, base 字段不变
 *
 * @details
 * 本章演化点全部集中在 led.h: 引入 struct led_ops 把多个函数指针
 * 打包成一张表. base 这一层故意先不动 -- ch09 § 9.4 / § 9.5.6 解释
 * 了为什么"打包"和"放进对象"分两章讲: 一次只解决一件事, 认知负荷小.
 *
 * 把 ops 字段塞进 base 是 ch10 (vptr 落地) 的工作. 见 ch10 § 10.3.
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

#endif /* LED_BASE_H */
