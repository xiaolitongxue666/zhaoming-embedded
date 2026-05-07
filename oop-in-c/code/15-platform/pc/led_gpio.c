/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.c - 子类一: GPIO LED 实现 (ch15 完整版, 风格 A)
 *
 * 这一份只负责 GPIO 子类: 三件套实现 (gpio_on / gpio_off, set_brightness
 * 故意不填走父类默认) + ops 表 (gpio_ops, file-static const) +
 * led_gpio_init 构造函数.
 *
 * 子类实现层函数签名都是 (struct led_base *me) -- 父类统一接口透过 ops
 * 跳过来时, 拿到的是 base 指针. 第一行用 container_of 反推回子类拿硬件
 * 字段 (pin / on_level). 见 ch13 § 13.5 三步宏 container_of.
 *
 * 这一层永远只调 platform 层的封装函数 (platform_gpio_init / write,
 * 在 common/platform.h 声明), 从来不直接碰寄存器. 同一份 led_gpio.c
 * 在 PC、STM32、Linux 上都能编译运行, 源码 0 修改.
 *
 * 见 ch15 § 15.3 子类层.
 */

#include "led_gpio.h"
#include "container_of.h"
#include "platform.h"
#include <stdio.h>

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	printf("  [%s] led_on  -> GPIO Pin%u\n",
	       me->name, (unsigned)self->pin);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, !self->on_level);
	me->is_on = false;
	printf("  [%s] led_off -> GPIO Pin%u\n",
	       me->name, (unsigned)self->pin);
	return 0;
}

/* set_brightness 故意不填, GPIO 不支持调光, 走父类默认行为 */
static const struct led_ops gpio_ops = {
	.on  = gpio_on,
	.off = gpio_off,
};

int led_gpio_init(struct led_gpio *me, const char *name,
                  uint8_t pin, bool on_level)
{
	int rc;
	if (!me)
		return -1;
	rc = led_base_init(&me->base, name, &gpio_ops);
	if (rc != 0)
		return rc;
	me->pin      = pin;
	me->on_level = on_level;
	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, !on_level);    /* 上电先关灯 */
	return 0;
}
