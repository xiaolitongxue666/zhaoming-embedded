/* SPDX-License-Identifier: MIT */
/**
 * @file  led_gpio.c
 * @brief LED GPIO 子类实现 + led_ops_gpio 操作表
 *
 * @details
 * 三个 static 函数 gpio_on / gpio_off / gpio_toggle 是这种 LED 的具体
 * 实现. 它们的外部入口只有 led_ops_gpio 这张表, static 隐藏的不是
 * 数据, 是 API 表面.
 *
 * 函数签名都是 (struct led_base *me) -- ops 表里函数指针的参数类型
 * 是基类指针, 不是某个具体子类. 同一种行为接口要被不同子类共用,
 * 参数类型必须收敛到 base, 否则一种 ops 表只能服务一个子类, 失去
 * 打包的意义.
 *
 * 函数体里 (struct led_gpio *)me 强转回各自子类拿硬件字段 (pin).
 * 这一招的前提是 base 在子类的第 0 偏移 -- C 标准保证 struct 第一个
 * 字段地址就是 struct 自身地址, 强转回去拿到同一块内存.
 */

#include "led_gpio.h"
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
 * const + 全局 -> 编译期 designated initializer 把每个字段填好 ->
 * 链接器把这两个对象放进 .rodata 段 -> 运行时只读, 改写直接 SIGSEGV
 * 或 MCU HardFault. 这一层防御是工业代码的硬要求 (见 ch09 § 9.5.3).
 *
 * designated initializer .on = ... 这种写法 (C99): 不依赖字段顺序,
 * 哪天调换 struct 字段, 这里不用改. 没列出的字段自动填 0/NULL
 * (C99 § 6.7.8 第 21 段). 见 ch09 § 9.5.1.
 */
const struct led_ops led_ops_gpio = {
	.on     = gpio_on,
	.off    = gpio_off,
	.toggle = gpio_toggle,
};
