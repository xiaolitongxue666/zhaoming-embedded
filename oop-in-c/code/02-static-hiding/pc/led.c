/* SPDX-License-Identifier: MIT */
/*
 * led.c - LED 模块实现（后厨）
 *
 * struct led 的字段定义只在这个文件里。led.h 只看得到 struct led;
 * 这个 forward declaration，所以外部代码：
 *
 *   struct led *me = led_create(13);
 *   me->pin = 999;       <-- 编译报错: invalid use of undefined type
 *
 * 同时，本文件内部用到的工具函数（update_hardware / brightness_valid）
 * 都加 static，外部链接器看不到它们，进一步关闭了一道门。
 */

#include "led.h"
#include <stdio.h>
#include <stdlib.h>

/* 字段定义：只在 led.c 内可见 */
struct led {
	uint8_t pin;            /* GPIO 引脚号 */
	uint8_t brightness;     /* 当前亮度 0~100 */
	bool    is_on;          /* 当前开关状态 */
};

/*
 * static 辅助函数：file-private（文件私有），别的 .c 看不到也调不到。
 * 集中处理硬件操作，这样 led_on / led_off / led_set_brightness 都走
 * 同一条路径，将来要改硬件逻辑（比如低电平点亮）只改一个地方。
 */
static void update_hardware(struct led *me)
{
	platform_gpio_write(me->pin, me->is_on);
}

static bool brightness_valid(uint8_t brightness)
{
	return brightness <= 100;
}

/* ---- 工厂函数 ---- */

struct led *led_create(uint8_t pin)
{
	struct led *me = malloc(sizeof(*me));
	if (!me)
		return NULL;

	me->pin = pin;
	me->brightness = 0;
	me->is_on = false;

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	update_hardware(me);

	printf("  [LED] Pin%u initialized\n", (unsigned)pin);
	return me;
}

void led_destroy(struct led *me)
{
	if (!me)
		return;

	me->is_on = false;
	update_hardware(me);
	platform_gpio_deinit(me->pin);

	printf("  [LED] Pin%u released\n", (unsigned)me->pin);
	free(me);
}

/* ---- 操作函数 ---- */

int led_on(struct led *me)
{
	if (!me)
		return -1;

	me->is_on = true;
	update_hardware(me);

	printf("  [LED] Pin%u ON\n", (unsigned)me->pin);
	return 0;
}

int led_off(struct led *me)
{
	if (!me)
		return -1;

	me->is_on = false;
	update_hardware(me);

	printf("  [LED] Pin%u OFF\n", (unsigned)me->pin);
	return 0;
}

int led_toggle(struct led *me)
{
	if (!me)
		return -1;

	if (me->is_on)
		led_off(me);
	else
		led_on(me);

	return 0;
}

int led_set_brightness(struct led *me, uint8_t brightness)
{
	if (!me)
		return -1;
	if (!brightness_valid(brightness)) {
		printf("  [LED] Error: brightness %u out of range (0~100)\n",
		       (unsigned)brightness);
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
	if (!me)
		return -1;

	if (is_on)
		*is_on = me->is_on;
	if (brightness)
		*brightness = me->brightness;

	return 0;
}
