/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.c
 * @brief led_base 共有 init 实现 (ch14 版, 沿用 ch10/ch13 一字不变)
 *
 * @details
 * 子类 init (led_gpio_init / led_pwm_init) 第一行调 led_base_init, 把
 * 对应的 const ops 表作为常量传进来. 共有字段 (ops / name / is_on)
 * 一次填好, 子类 init 后续只填自己的硬件字段 (pin / channel / on_level
 * 等).
 *
 * 这一章 (ch14) 所有变化在 led.c 里 (父类统一接口里加三种 NULL 处理
 * 策略), led_base.c 跟 ch10/ch11/ch13 一字不动.
 */

#include "led_base.h"
#include <stdio.h>

int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops)
{
	if (!me || !name || !ops)
		return -1;

	me->ops   = ops;
	me->name  = name;
	me->is_on = false;

	printf("  [base] \"%s\" common init done, ops=%p\n",
	       name, (const void *)ops);
	return 0;
}
