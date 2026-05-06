/* SPDX-License-Identifier: MIT */
/*
 * led.h - LED 模块对外接口
 *
 * 见书 ch03 § 3.4 名字自带说明书 + § 3.5 这就是一个完整的"类"。
 *
 * ch03 在 ch01-ch02 的基础上把命名规范固定下来：
 *   - 文件命名：led.h + led.c
 *   - 类型命名：struct led
 *   - 函数前缀：led_xxx        <- 链接器看到的全局符号唯一, 跟同
 *                                 项目里的 motor_init 撞不到
 *   - 生命周期函数：led_init (构造) + led_deinit (析构)
 *
 * 前缀 + init/deinit + 一对 .h/.c, 这一组就是 C 圈子事实上的
 * "class" 写法。Linux 内核 (kref_init / kref_get / kref_put)、
 * FreeRTOS (xQueueCreate / xQueueSend / xQueueReceive)、glibc
 * 全部按这一套写, ch05 会逐项映射到 STM32 HAL 库源码上验证。
 *
 * C++ 的 namespace + 类名做的就是这件事 (name mangling), 编译器
 * 替你操作字符串。C 里你手写, 看得见每一个细节。
 *
 * 字段从 ch01 起一直在 .h 公开。ch02 已经讲过 static 锁内部工具
 * 函数 + "private" 字段注释 + 命名纪律这一套软 + 硬 private 组合。
 * 这一章在此基础上把命名规范固定下来 (前缀 + init/deinit), 工程纪律
 * 是"通过 API 改"，不是"能改就改"。主流 C 项目 (nginx / Redis / LVGL
 * / Linux 内核大部分驱动) 都走这条路。
 */

#ifndef LED_H
#define LED_H

#include "platform.h"

/* 引脚有效范围（教学用，真实工程按 SoC 引脚数定） */
#define LED_PIN_MAX 31

struct led {
	uint8_t pin;            /* GPIO 引脚号 */
	uint8_t brightness;     /* 当前亮度 0~100 */
	bool    is_on;          /* 当前开关状态 */
	bool    initialized;    /* 是否已 led_init */
};

/* 生命周期 */
int led_init(struct led *me, uint8_t pin);
int led_deinit(struct led *me);

/* 操作 */
int led_on(struct led *me);
int led_off(struct led *me);
int led_toggle(struct led *me);
int led_set_brightness(struct led *me, uint8_t brightness);

/* 查询 */
int led_get_state(const struct led *me, bool *is_on, uint8_t *brightness);

#endif /* LED_H */
