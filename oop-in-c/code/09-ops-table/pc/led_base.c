/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.c
 * @brief led_base_init 公共初始化 + test_led 通用测试入口实现
 *
 * @details
 * 本章 base 这一层故意先不动 -- 一次只解决一件事, 认知负荷小. ops 表
 * 的引入集中在子类的 .c (led_gpio.c / led_pwm.c) 里, 各自填一张表.
 *
 * test_led 函数体走 ops->on / ops->off / ops->toggle 三个名字访问,
 * 不会因为传参顺序传反翻车. 三个函数指针都查 NULL: 某种 LED 不支持
 * 某个动作时 ops->xxx 可能没填, 调用前必须查 (见 ch09 § 9.5.2).
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

int test_led(struct led_base *me, const struct led_ops *ops)
{
	if (!me || !ops || !ops->on || !ops->off || !ops->toggle)
		return -1;

	printf("  [test] open ...\n");
	ops->on(me);
	printf("  [test] toggle ...\n");
	ops->toggle(me);
	printf("  [test] close ...\n");
	ops->off(me);
	return 0;
}
