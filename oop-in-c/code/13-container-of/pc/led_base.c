/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.c
 * @brief led_base 共有 init 实现 (ch13 版)
 *
 * @details
 * 子类的 init (led_gpio_init / led_pwm_init / led_i2c_init) 第一行调
 * led_base_init, 把对应的 const ops 表作为常量传进来. 共有字段
 * (ops / name / is_on) 一次填好, 子类 init 后续只填自己的硬件字段
 * (pin / channel / bus / addr 等).
 *
 * 跟 ch10/ch11 是完全一样的接口. ch13 这一章的所有变化在 led.c 里
 * (子类实现层用 container_of 而不是强转), led_base.c 一行不动.
 */

#include "led_base.h"
#include <stdio.h>

int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops)
{
	if (!me || !name || !ops)
		return -1;

	me->ops = ops;
	me->name = name;
	me->is_on = false;

	printf("  [base] \"%s\" common init done, ops=%p\n",
	       name, (const void *)ops);
	return 0;
}
