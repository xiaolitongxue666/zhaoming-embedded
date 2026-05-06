/* SPDX-License-Identifier: MIT */
/*
 * led.c - LED 模块实现（后厨）
 *
 * 内部工具函数 update_hardware / brightness_valid 加 static, 链接器
 * 只把它们写成 file-local 符号，外部 .c 文件根本看不到名字，调用就报
 * undefined reference。这是机制层硬锁。
 *
 * 字段定义还在 led.h 里公开，看 led.h 的人能看到 pin / brightness /
 * is_on。靠 "private" 注释 + 命名纪律 + code review 让外部别直接
 * 写字段。这是工程层软锁。
 *
 * 两层组合起来的效果，和 C++ 的 private 等价。运行时一行 cost 都没多。
 */

#include "led.h"
#include <stdio.h>

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

/* ---- 生命周期 ---- */

int led_init(struct led *me, uint8_t pin)
{
	if (!me)
		return -1;

	me->pin = pin;
	me->brightness = 0;
	me->is_on = false;

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	update_hardware(me);

	printf("  [LED] Pin%u initialized\n", (unsigned)pin);
	return 0;
}

int led_deinit(struct led *me)
{
	if (!me)
		return -1;

	me->is_on = false;
	update_hardware(me);
	platform_gpio_deinit(me->pin);

	me->brightness = 0;

	printf("  [LED] Pin%u released\n", (unsigned)me->pin);
	return 0;
}

/* ---- 操作 ---- */

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

/* ---- 查询 ---- */

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
