/* SPDX-License-Identifier: MIT */
/**
 * @file  led.h
 * @brief LED 父类统一接口 - 向上转型工程化高潮
 *
 * @details
 * 本章 (ch12) 把前面演化好的 ops 表机制放进真实工程结构. 这一份
 * led.h 在 ch12 起退到 "公开接口集中点" 的角色: 应用层只 #include
 * 这一个头文件就拿到 struct led_ops + led_on / led_off + struct
 * led_base, 不需要碰子类头文件.
 *
 * 各子类 (GPIO / PWM / I2C) 单独一对 .h / .c 文件, 子类头文件只在
 * board_init.c 这种"认识硬件"的位置 include. 这是 Linux 内核和工业
 * 项目的常见组织方式 -- "每个子类一个文件" 让 git blame 清晰、ABI
 * 改动定位方便.
 *
 * 关键 ABI 不变量 (见 ch12 § 12.2, C99 § 6.7.2.1):
 *   "结构体第一个成员的地址等于结构体本身的地址."
 * 这一条让
 *     struct led_base *handle = &gpio_led.base;
 * 合法 (类型对齐) 又零开销 (偏移 0, 不生成加法指令). 这是整章
 * 所有威力的根基.
 *
 * 应用层入口只有两个函数: led_on / led_off, 都接 struct led_base *
 * 句柄. 应用层一行硬件字样都没有.
 */

#ifndef LED_H
#define LED_H

#include "led_base.h"

/*
 * struct led_ops - 操作表 (ops 表). 子类把自己的实现填进去,
 * 父类统一接口 led_on / led_off 通过 ops 表分发到子类.
 *
 * ch12 主题是向上转型, 应用层只用得着 on / off, 表先收缩成两字段.
 * 后续章节按需要再加字段.
 */
struct led_ops {
	int (*on)(struct led_base *me);
	int (*off)(struct led_base *me);
};

/* 应用层入口: 所有调用都走 led_base 句柄 */
int led_on(struct led_base *me);
int led_off(struct led_base *me);

#endif /* LED_H */
