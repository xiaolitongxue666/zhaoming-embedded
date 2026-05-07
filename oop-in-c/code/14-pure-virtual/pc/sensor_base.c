/* SPDX-License-Identifier: MIT */
/**
 * @file  sensor_base.c
 * @brief sensor_base 共有 init 实现
 *
 * @details
 * 风格和 led_base_init 一字不变. 这一章 sensor 体系所有"接口风格"的
 * 变化都在 sensor.c 里 (父类统一接口三件套全部 assert), sensor_base.c
 * 只负责"接 ops"这一件事.
 */

#include "sensor_base.h"
#include <stdio.h>

int sensor_base_init(struct sensor_base *me, const char *name,
                     const struct sensor_ops *ops)
{
	if (!me || !name || !ops)
		return -1;

	me->ops  = ops;
	me->name = name;

	printf("  [base] \"%s\" sensor common init done, ops=%p\n",
	       name, (const void *)ops);
	return 0;
}
