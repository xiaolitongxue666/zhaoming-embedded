/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.c
 * @brief led_base 通用 init + test_led 通用测试入口实现
 *
 * @details
 * 子类的 init (led_gpio_init / led_pwm_init) 第一行调 led_base_init,
 * 把"我用哪张 ops 表"作为常量参数传进来. 基类把 ops 存到 me->ops 字段.
 * 一次填好, 对象之后不用再碰 ops.
 *
 * test_led 接 base 指针就够了. ops 表跟着 me 自己跑, 函数体里直接走
 * me->ops->on(me). 同一份 test_led, 不同 LED 不同行为.
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

int test_led(struct led_base *me)
{
	if (!me || !me->ops ||
	    !me->ops->on || !me->ops->off || !me->ops->toggle)
		return -1;

	printf("  [test] open ...\n");
	me->ops->on(me);
	printf("  [test] toggle ...\n");
	me->ops->toggle(me);
	printf("  [test] close ...\n");
	me->ops->off(me);
	return 0;
}
