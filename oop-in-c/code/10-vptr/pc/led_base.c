/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.c
 * @brief vptr 落地的基类实现
 *
 * @details
 * 子类的 init (led_gpio_init / led_pwm_init) 第一行调 led_base_init,
 * 把"我用哪张 ops 表"作为常量参数传进来. 基类把 ops 存到 me->ops 字段,
 * 一次填好, 对象一辈子不用改 ops.
 *
 * 这一步等价于 C++ 编译器在 class led_gpio : public led_base 的对象
 * 构造时, 自动把 vptr 设成 &led_gpio::vtable. 你 C 里手动写, 机器码
 * 几乎一字不差. 见 ch10 § 10.7.
 *
 * 对外只保留 led_base_get_name 一个查询函数. 真正的 dispatch
 * (led_on / led_off / led_toggle) 在 led.c 里 -- 那才是基类层多态
 * 调用的核心入口.
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
