/* SPDX-License-Identifier: MIT */
/*
 * led.h - LED 模块对外接口
 *
 * 见书 ch04 § 4.5 改造全貌 · 数据三级归位 + § 4.7.2 三种持有方式。
 *
 * ch04 把数据归位三步走做完：
 *   1. 实例数据（pin / brightness / is_on）-> struct 字段，跟着 me 走
 *      每个 LED 自己持有, 红绿不互相覆盖 (修掉 led_bad.c 的 g_pin bug)
 *   2. 模块共享数据（init 计数 / debug 开关）-> static 变量，文件私有
 *      外部 extern 找不到, 谁也偷偷改不了
 *   3. 只读常量（亮度上限 / 引脚上限）-> static const，编译期不可改
 *      MAX_BRIGHTNESS = 0; 编译器直接报 read-only variable
 *
 * 这一章用静态对象池演示 (led_pool[8]), 把"模块共享数据 + 实例数据"
 * 凑齐让三类归位一起呈现。**对象池不是 LED 的唯一答案** —— LED 这种
 * 数量固定的全局对象, 直接 static struct led red_led; 给每颗一个
 * 静态实例更常见 (见书 § 4.7.2 写法 A)。生命周期不固定的对象用 RTOS
 * 提供的 heap (FreeRTOS heap_4 / RT-Thread rt_malloc)。三种持有方式
 * 按对象生命周期选, 不是 "先静态再说"。
 */

#ifndef LED_H
#define LED_H

#include "platform.h"

#define LED_POOL_SIZE 8

struct led {
	uint8_t pin;            /* GPIO 引脚号 */
	uint8_t brightness;     /* 当前亮度 0~100 */
	bool    is_on;          /* 当前开关状态 */
	bool    in_use;         /* 对象池槽位是否被占用 */
};

/* 工厂函数：从静态池里取一个槽，绝不 malloc */
struct led *led_acquire(uint8_t pin);
void        led_release(struct led *me);

/* 操作 */
int led_on(struct led *me);
int led_off(struct led *me);
int led_toggle(struct led *me);
int led_set_brightness(struct led *me, uint8_t brightness);

/* 查询 */
int led_get_state(const struct led *me, bool *is_on, uint8_t *brightness);

/* 模块级查询：通过函数访问 static 数据 */
unsigned int led_get_init_count(void);

#endif /* LED_H */
