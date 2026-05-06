/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.c
 * @brief led_base 通用 init: ops 一次填好, 对象一辈子不用改
 *
 * @details
 * 子类的 init (led_gpio_init / led_pwm_init) 第一行调 led_base_init,
 * 把"我用哪张 ops 表"作为常量参数传进来. 基类把 ops 存到 me->ops 字段.
 * 一次填好, 对象之后不用再碰 ops.
 *
 * 对外只保留 led_base_get_name 一个查询函数. 真正的调用入口在 led.c.
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

const char *led_base_get_name(const struct led_base *me)
{
	if (!me)
		return "(null)";
	return me->name;
}
