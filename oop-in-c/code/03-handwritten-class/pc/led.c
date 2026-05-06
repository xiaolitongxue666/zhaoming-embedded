/* SPDX-License-Identifier: MIT */
/*
 * led.c - LED 模块实现
 *
 * 命名规范固定：
 *   函数前缀 led_xxx, struct led, led.h + led.c。
 *
 * 生命周期：led_init 第一个调，led_deinit 最后调。中间随便用。
 * led_init 做三件事：参数校验 / 硬件初始化 / 默认状态。
 * led_deinit 做两件事：关硬件 / 释放资源。
 *
 * initialized 标志位拦截"没 init 就用"的常见错误。函数返回 -3
 * 提醒调用方"先 led_init"，比硬件死锁好排查得多。
 */

#include "led.h"
#include <stdio.h>

/* ---- file-private 工具函数 ---- */

static void update_hardware(struct led *me)
{
	platform_gpio_write(me->pin, me->is_on);
}

static bool brightness_valid(uint8_t brightness)
{
	return brightness <= 100;
}

static bool pin_valid(uint8_t pin)
{
	return pin <= LED_PIN_MAX;
}

/* ---- 生命周期 ---- */

int led_init(struct led *me, uint8_t pin)
{
	if (!me)
		return -1;
	if (!pin_valid(pin)) {
		printf("  [LED] Error: pin %u out of range (0~%u)\n",
		       (unsigned)pin, (unsigned)LED_PIN_MAX);
		return -2;
	}

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);

	me->pin = pin;
	me->brightness = 0;
	me->is_on = false;
	me->initialized = true;

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
	me->initialized = false;

	printf("  [LED] Pin%u released\n", (unsigned)me->pin);
	return 0;
}

/* ---- 操作 ---- */

int led_on(struct led *me)
{
	if (!me)
		return -1;
	if (!me->initialized) {
		printf("  [LED] Error: not initialized, call led_init first\n");
		return -3;
	}

	me->is_on = true;
	update_hardware(me);

	printf("  [LED] Pin%u ON\n", (unsigned)me->pin);
	return 0;
}

int led_off(struct led *me)
{
	if (!me)
		return -1;
	if (!me->initialized) {
		printf("  [LED] Error: not initialized, call led_init first\n");
		return -3;
	}

	me->is_on = false;
	update_hardware(me);

	printf("  [LED] Pin%u OFF\n", (unsigned)me->pin);
	return 0;
}

int led_toggle(struct led *me)
{
	if (!me)
		return -1;
	if (!me->initialized) {
		printf("  [LED] Error: not initialized, call led_init first\n");
		return -3;
	}

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
	if (!me->initialized) {
		printf("  [LED] Error: not initialized, call led_init first\n");
		return -3;
	}
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
