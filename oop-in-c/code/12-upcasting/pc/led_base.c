/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.c
 * @brief led_base 通用 init - ops / name / is_on 一次填好
 *
 * @details
 * 子类 init (led_gpio_init / led_pwm_init / led_i2c_init) 第一行调
 * led_base_init, 把 "我用哪张 ops 表" 作为常量参数传进来. 父类把
 * ops / name / is_on 三个公共字段一次填好, 之后子类只管自己的硬件
 * 资源 (pin / channel / addr).
 *
 * NULL check 都集中在这里. 子类 init 拿到非 0 返回值就把错误码原样
 * 往上抛.
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
