/* SPDX-License-Identifier: MIT */
/*
 * led.c - LED 模块实现，数据归位完成形态
 *
 * 见书 ch04 § 4.5 改造全貌 · 数据三级归位。
 *
 * 文件开头没有一个裸露的 int g_xxx 全局变量。每一份数据都有它该
 * 呆的地方 —— 这就是数据所有权 (data ownership)：
 *
 * 实例数据    -> struct led 字段（跟着 me 走，每个 LED 自己持有）
 *               pin / brightness / is_on 红绿两灯各一份, 不会互相
 *               覆盖（对照 led_bad.c 的 g_pin bug）。
 *
 * 模块数据    -> static 变量（文件私有）
 *               s_init_count   累计 acquire 次数
 *               s_debug_flag   调试开关
 *               led_pool       静态对象池
 *               static 让别的 .c 文件 extern 都看不到, 谁也偷偷
 *               改不了 (见 ch02 § 2.6.5 链接器视角)。
 *
 * 只读常量    -> static const（编译期不可改）
 *               MAX_BRIGHTNESS / MAX_PIN
 *               MAX_BRIGHTNESS = 0; 编译器直接报 assignment of
 *               read-only variable, 运行时零开销。比 #define 多
 *               一份类型检查 + 调试器可见。
 *
 * 调用 led_acquire 从池里取空闲槽, 不 malloc。MCU 上零堆碎片、
 * O(1) 分配 / 释放。led_pool 在 .bss 段, 上电由 startup 清零,
 * 所以每个 in_use 一开机就是 false 可以直接用 (见 § 4.7.5)。
 *
 * 数据没有主人, bug 就是主人。
 */

#include "led.h"
#include <stdio.h>
#include <string.h>

/* ---- 第三类：只读常量 -> static const ---- */
static const uint8_t MAX_BRIGHTNESS = 100;
static const uint8_t MAX_PIN        = 31;

/* ---- 第二类：模块共享数据 -> static 变量 ---- */
static struct led led_pool[LED_POOL_SIZE];
static unsigned int s_init_count;
static int          s_debug_flag;

/* ---- file-private 工具函数 ---- */

static void update_hardware(struct led *me)
{
	platform_gpio_write(me->pin, me->is_on);
}

static bool brightness_valid(uint8_t brightness)
{
	return brightness <= MAX_BRIGHTNESS;
}

static bool pin_valid(uint8_t pin)
{
	return pin <= MAX_PIN;
}

static void debug_print(const char *msg)
{
	if (s_debug_flag)
		printf("  [LED-DEBUG] %s\n", msg);
}

/* ---- 工厂：从静态池里取槽，不 malloc ---- */

struct led *led_acquire(uint8_t pin)
{
	if (!pin_valid(pin)) {
		printf("  [LED] Error: pin %u out of range (0~%u)\n",
		       (unsigned)pin, (unsigned)MAX_PIN);
		return NULL;
	}

	for (size_t i = 0; i < LED_POOL_SIZE; i++) {
		struct led *me = &led_pool[i];
		if (me->in_use)
			continue;

		me->pin = pin;
		me->brightness = 0;
		me->is_on = false;
		me->in_use = true;

		platform_gpio_init(pin, GPIO_MODE_OUTPUT);
		update_hardware(me);

		s_init_count++;
		debug_print("acquired");
		printf("  [LED] Pin%u acquired (slot %zu, total inits %u)\n",
		       (unsigned)pin, i, s_init_count);
		return me;
	}

	printf("  [LED] Error: pool exhausted (size=%d)\n", LED_POOL_SIZE);
	return NULL;
}

void led_release(struct led *me)
{
	if (!me)
		return;

	me->is_on = false;
	update_hardware(me);
	platform_gpio_deinit(me->pin);

	debug_print("released");
	printf("  [LED] Pin%u released\n", (unsigned)me->pin);

	me->pin = 0;
	me->brightness = 0;
	me->in_use = false;
}

/* ---- 操作 ---- */

int led_on(struct led *me)
{
	if (!me || !me->in_use)
		return -1;

	me->is_on = true;
	update_hardware(me);

	printf("  [LED] Pin%u ON\n", (unsigned)me->pin);
	return 0;
}

int led_off(struct led *me)
{
	if (!me || !me->in_use)
		return -1;

	me->is_on = false;
	update_hardware(me);

	printf("  [LED] Pin%u OFF\n", (unsigned)me->pin);
	return 0;
}

int led_toggle(struct led *me)
{
	if (!me || !me->in_use)
		return -1;

	if (me->is_on)
		led_off(me);
	else
		led_on(me);

	return 0;
}

int led_set_brightness(struct led *me, uint8_t brightness)
{
	if (!me || !me->in_use)
		return -1;
	if (!brightness_valid(brightness)) {
		printf("  [LED] Error: brightness %u out of range (0~%u)\n",
		       (unsigned)brightness, (unsigned)MAX_BRIGHTNESS);
		return -2;
	}

	me->brightness = brightness;
	me->is_on = (brightness > 0);
	update_hardware(me);

	printf("  [LED] Pin%u brightness set to %u%%\n",
	       (unsigned)me->pin, (unsigned)brightness);
	return 0;
}

int led_get_state(const struct led *me, bool *is_on, uint8_t *brightness)
{
	if (!me || !me->in_use)
		return -1;

	if (is_on)
		*is_on = me->is_on;
	if (brightness)
		*brightness = me->brightness;

	return 0;
}

unsigned int led_get_init_count(void)
{
	return s_init_count;
}
