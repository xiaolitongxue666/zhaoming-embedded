/* SPDX-License-Identifier: MIT */
/*
 * led.c - LED GPIO 子类的实现
 *
 * 见书 ch06 § 6.4 子类构造函数链。
 *
 * led_gpio_init 第一行调 led_base_init, 把基类部分初始化掉.
 * 然后再处理 GPIO 子类自己的字段 (pin).
 *
 * &me->base 这个写法是关键: me 是 struct led_gpio *,
 * me->base 是结构体字段 (类型 struct led_base), &me->base 是这个
 * 字段的地址。把它递给 led_base_init, 基类只看自己接到的部分,
 * 不知道也不需要知道外层是哪个子类。基类只看接口部分, 子类只看
 * 自己的字段, 边界清晰。
 *
 * C++ 的 led_gpio g; 编译器自动调一遍 led_base 的构造函数再调
 * led_gpio 的构造函数, 就是这里手写的事情。编译器帮你做了。
 */

#include "led.h"
#include <stdio.h>

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin)
{
	int rc;

	if (!me)
		return -1;

	rc = led_base_init(&me->base, name);
	if (rc != 0)
		return rc;

	me->pin = pin;
	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, false);

	printf("  [GPIO] sub-class init done (pin=%u)\n", (unsigned)pin);
	return 0;
}

int led_gpio_on(struct led_gpio *me)
{
	if (!me)
		return -1;

	me->base.is_on = true;
	platform_gpio_write(me->pin, true);
	printf("  [GPIO] \"%s\" ON\n", me->base.name);
	return 0;
}

int led_gpio_off(struct led_gpio *me)
{
	if (!me)
		return -1;

	me->base.is_on = false;
	platform_gpio_write(me->pin, false);
	printf("  [GPIO] \"%s\" OFF\n", me->base.name);
	return 0;
}
