/* SPDX-License-Identifier: MIT */
/**
 * @file  led_gpio.c
 * @brief GPIO 子类 init + 实现层 + led_ops_gpio 操作表 (ch12 版)
 *
 * @details
 * gpio_on / gpio_off 函数签名都是 (struct led_base *me) -- 父类统一
 * 接口 led_on(base) 透过 ops 指针跳过来时, 拿到的是 base 指针.
 * 函数体里 (struct led_gpio *)me 强转回子类拿 pin / on_level 字段.
 * 这一招的前提是 base 在子类的第 0 偏移 (向上转型不变量, 见 § 12.2).
 * ch13 会用 container_of 替掉强转, ch12 还是强转风格.
 *
 * gpio_ops 表用 static + const + designated initializer 三件套, 落进
 * .rodata 段: MCU 上烧到 Flash 不占 RAM, 100 颗同类型 GPIO LED 共享
 * 同一张表, 链接期不可改防止运行时被踩成野指针.
 */

#include "led_gpio.h"
#include "platform.h"
#include <stdio.h>

/*
 * 实现层接 struct led_base *me. 第一行 (struct led_gpio *)me 强转
 * 回子类拿到 pin 字段. 合法因为 base 在 led_gpio 的第 0 字段
 * (向上转型不变量, 见 ch12 § 12.2).
 */
static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;     /* 反推回子类 */
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	printf("  [%s] GPIO Pin%u ON\n", me->name, (unsigned)self->pin);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	platform_gpio_write(self->pin, !self->on_level);
	me->is_on = false;
	printf("  [%s] GPIO Pin%u OFF\n", me->name, (unsigned)self->pin);
	return 0;
}

static const struct led_ops led_ops_gpio = {
	.on  = gpio_on,
	.off = gpio_off,
};

/*
 * 子类构造函数 - 第一行调 led_base_init 把 ops 表交给父类,
 * 之后填子类自己的硬件资源, 最后调 platform 层把外设拉起来.
 *
 * platform_gpio_write(pin, !on_level) 在 init 末尾把灯先关掉
 * (避免上电瞬间灯就亮个莫名其妙).
 */
int led_gpio_init(struct led_gpio *me, const char *name,
                  uint8_t pin, bool on_level)
{
	int rc;
	if (!me)
		return -1;
	rc = led_base_init(&me->base, name, &led_ops_gpio);
	if (rc != 0)
		return rc;
	me->pin = pin;
	me->on_level = on_level;
	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, !on_level);    /* 上电先关灯 */
	return 0;
}
