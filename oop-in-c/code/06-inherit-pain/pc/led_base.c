/* SPDX-License-Identifier: MIT */
/*
 * led_base.c - 基类的实现
 *
 * led_base_init 只做基类自己的活: 把 name 字段填好, is_on 清成 false.
 * 子类的 init (led_gpio_init / led_pwm_init) 第一行调它, 把基类部分
 * 初始化掉, 剩下子类自己的硬件字段子类自己来.
 */

#include "led_base.h"
#include <stdio.h>

int led_base_init(struct led_base *me, const char *name)
{
	if (!me || !name)
		return -1;

	me->name = name;
	me->is_on = false;

	printf("  [base] \"%s\" common init done (is_on=false)\n", name);
	return 0;
}

const char *led_base_get_name(const struct led_base *me)
{
	if (!me)
		return "(null)";
	return me->name;
}

bool led_base_is_on(const struct led_base *me)
{
	if (!me)
		return false;
	return me->is_on;
}
