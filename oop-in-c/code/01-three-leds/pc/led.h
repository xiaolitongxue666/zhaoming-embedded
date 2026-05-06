/* SPDX-License-Identifier: MIT */
/*
 * led.h - 单 LED 抽象的最小接口
 *
 * 见书 ch01 § 1.3-1.4 把"区别"和"代码"分开。
 *
 * 一颗 LED 的全部数据放进 struct led，每个函数的第一个参数都是
 *   struct led *me
 * 表示"我现在操作的是哪一张挂号单"。这是封装最朴素的形态：
 * 同一份代码（led_on / led_off / ...）服务不同的数据（red / green / blue）。
 *
 * me 为什么是指针不是值传递：见书 § 1.7.1。简单说两点——
 * 值传递每次拷贝整个 struct（性能差），改也改不到原对象（语义错）。
 * C 的 me、C++ 的 this、Rust 的 &self，全部都是指针 / 引用。
 *
 * 这一章不讨论信息隐藏，所以 struct 字段对外可见，任何调用方都
 * 可以直接 me->pin = 999 把它弄坏 —— 这恰好是 ch02 要解决的问题。
 */

#ifndef LED_H
#define LED_H

#include "platform.h"

struct led {
	uint8_t pin;            /* GPIO 引脚号 */
	uint8_t brightness;     /* 当前亮度 0~100 */
	bool    is_on;          /* 当前开关状态 */
};

int led_init(struct led *me, uint8_t pin);
int led_deinit(struct led *me);
int led_on(struct led *me);
int led_off(struct led *me);
int led_toggle(struct led *me);
int led_set_brightness(struct led *me, uint8_t brightness);

#endif /* LED_H */
