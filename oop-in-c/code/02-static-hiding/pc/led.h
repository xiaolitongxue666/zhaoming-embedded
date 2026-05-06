/* SPDX-License-Identifier: MIT */
/*
 * led.h - LED 模块对外接口（契约）
 *
 * 见书 ch02 § 2.3 头文件契约 + § 2.4 信息隐藏。
 *
 * ch02 在 ch01 基础上的真实增量只有三件:
 *   1. struct led 字段每个挂 "private" 注释，明示外部别直接写
 *   2. led.c 里的 update_hardware / brightness_valid 加 static，
 *      链接期就让外部看不到这些内部工具函数
 *   3. 加 led_get_state API: 外部要读字段也走 API，不直接读 me->is_on
 *
 * 字段还在 .h 公开，没有藏到 .c。这是 C 圈子主流做法 (nginx / Redis /
 * LVGL / FreeRTOS / Linux 内核大部分驱动)。字段公开是为了让 ch06 起
 * 子类能把 struct led_base base 嵌进自己的第一个字段，做继承。字段藏 .c
 * 之后子类源文件直接编译报错。
 *
 * 拦截 me->pin = 999 这一行靠的是工程纪律: "private" 注释 + 命名
 * 一致 + code review。三道关一起上，业内 99% 的 C 项目这么做。完全靠
 * 编译器拦的方案叫不透明指针 (FILE * / pthread_t / sqlite3 *)，是跨
 * 二进制库 ABI 边界的写法，本书 OOP 主线 18 章不用。详见书 § 2.6.3。
 */

#ifndef LED_H
#define LED_H

#include "platform.h"

struct led {
	uint8_t pin;            /* private: 通过 led_init 设置 */
	uint8_t brightness;     /* private: 通过 led_set_brightness 设置 */
	bool    is_on;          /* private: 通过 led_on / led_off 设置 */
};

/* 生命周期 */
int led_init(struct led *me, uint8_t pin);
int led_deinit(struct led *me);

/* 操作 */
int led_on(struct led *me);
int led_off(struct led *me);
int led_toggle(struct led *me);
int led_set_brightness(struct led *me, uint8_t brightness);

/* 查询: 外部要读字段也走 API */
int led_get_state(const struct led *me, bool *is_on, uint8_t *brightness);

#endif /* LED_H */
