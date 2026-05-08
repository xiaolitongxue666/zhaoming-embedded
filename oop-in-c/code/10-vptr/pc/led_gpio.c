/* SPDX-License-Identifier: MIT */
/**
 * @file  led_gpio.c
 * @brief GPIO 子类 init + 实现层 + led_ops_gpio 操作表
 *
 * @details
 * led_gpio_init 把 &led_ops_gpio 这张 const 表交给 led_base_init,
 * base 把它存进 me->ops. 之后调用方拿到 base 指针走 me->ops->on(me),
 * 那张表跟着 me 自己跑.
 *
 * gpio_on / gpio_off / gpio_toggle 都声明成 static -- 它们是实现细节,
 * 应用层不该直接调. ops 表是它们对外的唯一入口. static 隐藏的不是
 * 数据, 是 API 表面.
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

/*
 * led_ops_gpio - GPIO 风格的操作表.
 *
 * const + 全局: 落 .rodata, 全程序唯一一份, 100 颗 GPIO LED 共享同一张
 * 12 字节的 ops 表. 链接器不准运行时被改成野指针.
 */
static const struct led_ops led_ops_gpio = {
	.on     = gpio_on,
	.off    = gpio_off,
	.toggle = gpio_toggle,
};
