/* SPDX-License-Identifier: MIT */
/*
 * led.h - LED 模块对外接口（菜单）
 *
 * 见书 ch02 § 2.3 头文件 = 菜单 + § 2.4 信息隐藏。
 *
 * ch02 的核心改动：
 *   ch01 里 struct led 的字段 pin / brightness / is_on 是公开的，
 *   任何调用方都能 me->pin = 999 把它弄坏 —— 同事改一行 LED 全乱。
 *
 *   这一章只做一件事：把字段定义从 led.h 搬到 led.c，
 *   led.h 里只留 struct led 的 forward declaration（前向声明）。
 *   外部代码看得到"有这么个类型"，但看不到字段，也就改不了 ——
 *   编译器在 me->pin = 999 这一行直接报
 *   "invalid use of undefined type"，0 漏网概率。
 *
 *   字段名一字不动（pin / brightness / is_on），改的只是 visibility。
 *
 * 一句话：
 *   .h 暴露能调的函数（菜单），.c 藏住数据细节（后厨）。
 *
 * 这种"看得到指针、看不到字段"的写法叫不透明指针（opaque pointer）。
 * libc FILE *、POSIX pthread_t、Win32 HANDLE、Linux 内核 struct file *
 * 都是这一形态，是工业级 C 库跨二进制边界的标准做法。
 */

#ifndef LED_H
#define LED_H

#include <stddef.h>
#include "platform.h"

/*
 * 前向声明：告诉编译器有这个类型，字段定义在 led.c 里。
 * 外部代码只能用 struct led * 作指针变量，不能定义 struct led 实例
 * （也读不到字段）。这就是 opaque pointer（不透明指针）的最朴素形态。
 */
struct led;

/* 工厂函数：分配并初始化一个 led 对象 */
struct led *led_create(uint8_t pin);
void        led_destroy(struct led *me);

/* 操作函数：第一个参数都是 me，对应"这是哪一颗 LED" */
int led_on(struct led *me);
int led_off(struct led *me);
int led_toggle(struct led *me);
int led_set_brightness(struct led *me, uint8_t brightness);

/*
 * 状态查询：外部不直接读字段，走这个 API。
 * is_on / brightness 任意一个传 NULL 表示"不关心这一项"。
 */
int led_get_state(const struct led *me, bool *is_on, uint8_t *brightness);

#endif /* LED_H */
