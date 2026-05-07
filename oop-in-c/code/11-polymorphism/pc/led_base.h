/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief led_base 字段集 + 父类统一接口 led_on / led_off / led_toggle
 *
 * @details
 * 字段集跟 ch10 一字不变. 本章 ch11 的演化点是: 在父类 led_base 这一
 * 层加 led_on / led_off / led_toggle 三个父类统一接口 (函数体一行
 * "return me->ops->on(me);"), 让应用层只对 base 指针编程, 不再写
 * me->ops->on(me) 这种长串.
 *
 * 公开接口从这一章起搬到 led_base.h 集中声明 -- 应用层只 #include
 * led_base.h 就够拿到 led_on / led_off / led_toggle, 不用直接 include
 * 子类头文件.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

/*
 * led_action_fn - 用 typedef 给函数指针类型起短名.
 */
struct led_base;
typedef int (*led_action_fn)(struct led_base *me);

/*
 * struct led_ops - 操作表.
 *
 * 把同一种 LED 的所有"可变行为"打包. 一种 LED 实现填一张表,
 * 父类层 led_on / led_off / led_toggle 走 me->ops->xxx(me) dispatch.
 */
struct led_ops {
	led_action_fn on;
	led_action_fn off;
	led_action_fn toggle;
};

struct led_base {
	const struct led_ops *ops;     /* 第一个字段, 对象起始地址处 */
	const char           *name;
	bool                  is_on;
};

int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);
const char *led_base_get_name(const struct led_base *me);

/*
 * 父类统一接口 - 写在 led_base.c, 所有子类共用.
 * 应用层只调 led_on / led_off / led_toggle, 看不到 ops 字段.
 */
int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_toggle(struct led_base *me);

#endif /* LED_BASE_H */
