/* SPDX-License-Identifier: MIT */
/*
 * led_base.c - 基类实现 (与 ch06 一致)
 */

#include "led_base.h"
#include <stdio.h>

int led_base_init(struct led_base *me, const char *name)
{
	if (!me || !name)
		return -1;

	me->name = name;
	me->is_on = false;

	printf("  [base] \"%s\" common init done\n", name);
	return 0;
}

const char *led_base_get_name(const struct led_base *me)
{
	if (!me)
		return "(null)";
	return me->name;
}
