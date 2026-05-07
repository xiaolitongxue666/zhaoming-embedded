/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.c - GPIO 子类 init + 实现层 + led_ops_gpio 操作表 (ch13 版)
 *
 * 子类实现层用 container_of 反推自己 -- 这是 ch13 主题. GPIO 子类
 * base 故意挪到偏移 4, 强转那一招 (struct led_gpio *)me 会算错 4 字节
 * 地址, container_of 一字不改还是对.
 *
 * gpio_on 内部读 self->magic 打印 0xCAFE 出来, 肉眼证明反推地址正确
 * (如果反推错 4 字节, 那个位置不再是 magic, 打出来就不是 0xCAFE).
 *
 * 见 ch13 § 13.6 在 gpio_on 里用一下 / § 13.8.2 位置无关 demo.
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
	printf("  [%s] GPIO Pin%u ON (magic=0x%04X)\n",
	       me->name, (unsigned)self->pin, (unsigned)self->magic);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, !self->on_level);
	me->is_on = false;
	printf("  [%s] GPIO Pin%u OFF\n", me->name, (unsigned)self->pin);
	return 0;
}

const struct led_ops led_ops_gpio = {
	.on  = gpio_on,
	.off = gpio_off,
	/* set_brightness 故意不填 -- GPIO 灯没有亮度概念 */
};

int led_gpio_init(struct led_gpio *me, const char *name,
		  uint8_t pin, bool on_level)
{
	int rc;
	if (!me)
		return -1;
	rc = led_base_init(&me->base, name, &led_ops_gpio);
	if (rc != 0)
		return rc;
	me->magic    = 0xCAFE;
	me->pin      = pin;
	me->on_level = on_level;

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, !on_level);     /* 上电先关灯 */
	return 0;
}
