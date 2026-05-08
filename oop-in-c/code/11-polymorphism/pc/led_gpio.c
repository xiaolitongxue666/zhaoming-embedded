/* SPDX-License-Identifier: MIT */
/**
 * @file  led_gpio.c
 * @brief GPIO 子类 init + 实现层 + led_ops_gpio 操作表
 *
 * @details
 * gpio_on / gpio_off / gpio_toggle 函数签名都是 (struct led_base *me) --
 * 父类统一接口 led_on(base) 透过 ops 指针跳过来时, 拿到的是 base 指针.
 * 函数体里 (struct led_gpio *)me 强转回子类拿 pin 字段. 这一招的前提
 * 是 base 在子类的第 0 偏移.
 */

#include "led_gpio.h"
#include <stdio.h>

static const struct led_ops led_ops_gpio;

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin)
{
	int rc;
	if (!me)
		return -1;
	rc = led_base_init(&me->base, name, &led_ops_gpio);
	if (rc != 0)
		return rc;
	me->pin = pin;
	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, false);
	printf("  [GPIO] sub-class init done (pin=%u)\n", (unsigned)pin);
	return 0;
}

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	me->is_on = true;
	platform_gpio_write(self->pin, true);
	printf("  [GPIO] \"%s\" ON\n", me->name);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	me->is_on = false;
	platform_gpio_write(self->pin, false);
	printf("  [GPIO] \"%s\" OFF\n", me->name);
	return 0;
}

static int gpio_toggle(struct led_base *me)
{
	if (me->is_on)
		return gpio_off(me);
	return gpio_on(me);
}

static const struct led_ops led_ops_gpio = {
	.on     = gpio_on,
	.off    = gpio_off,
	.toggle = gpio_toggle,
};
