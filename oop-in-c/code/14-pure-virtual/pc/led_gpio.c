/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.c - GPIO 子类 init + 实现层 + led_ops_gpio 操作表 (ch14 版)
 *
 * 子类实现里 (gpio_on / gpio_off) 用 ch13 学的 container_of 反推子类
 * 指针, 强转那一招在 ch13 故意挪 base 演示后已经退役.
 *
 * gpio_ops 只填 on / off, set_brightness 故意不填. C 标准下静态存储
 * 未显式初始化的字段被零初始化, 所以 led_ops_gpio.set_brightness 是
 * NULL. 这正是选填策略要演示的情形 -- 父类 led_set_brightness 看见
 * NULL 走默认 "安静跳过" 分支, 不崩.
 *
 * 见 ch14 § 14.3 选填策略.
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
	printf("  [%s] GPIO Pin%u ON\n", me->name, (unsigned)self->pin);
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

/*
 * GPIO 子类的 ops 表: 只填 on / off, set_brightness 故意不填.
 * C 标准下静态存储未显式初始化的字段被零初始化, 所以
 * led_ops_gpio.set_brightness 是 NULL. 这正是选填策略要演示的情形.
 *
 * static + const + designated initializer 的组合让这张表落进 .rodata 段:
 *   - MCU 上烧到 Flash, 完全不占 RAM
 *   - 100 颗同类型 GPIO LED 共享同一张表
 *   - 链接期不可改, 防止运行时被踩成野指针
 */
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

	me->pin      = pin;
	me->on_level = on_level;

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, !on_level);     /* 上电先关灯 */
	return 0;
}
