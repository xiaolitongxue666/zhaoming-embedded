/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.c
 * @brief 父类层: 共有 init + 父类统一接口 led_on / led_off / led_toggle
 *
 * @details
 * 父类做两件事:
 *   1) led_base_init - 共有字段 init (ops / name / is_on), 子类 init 调一次
 *   2) led_on / led_off / led_toggle - 父类统一接口, 函数体一行
 *      "return me->ops->on(me);" 三种子类共用
 *
 * 应用层只看到 led_on(base), 不直接碰 ops 字段, 也不需要知道这颗
 * LED 是 GPIO 还是 PWM.
 */

#include "led_base.h"
#include "led.h"
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

/*
 * 父类统一接口 - 函数体一行, 所有子类共用.
 *
 * led_on(base) 内部 me->ops->on(me): 红灯落到 gpio_on, 蓝灯落到
 * pwm_on, 绿灯落到 gpio_on. 应用层一行 led_on(base) 跑出三种行为.
 */
int led_on(struct led_base *me)
{
	if (!me || !me->ops || !me->ops->on)
		return -1;
	return me->ops->on(me);
}

int led_off(struct led_base *me)
{
	if (!me || !me->ops || !me->ops->off)
		return -1;
	return me->ops->off(me);
}

int led_toggle(struct led_base *me)
{
	if (!me || !me->ops || !me->ops->toggle)
		return -1;
	return me->ops->toggle(me);
}
