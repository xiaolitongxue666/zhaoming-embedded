/* SPDX-License-Identifier: MIT */
/*
 * led_base.c - LED 父类层 dispatch + 默认实现.
 *
 * 父类层在调用子类 ops 之前 assert 收拢必填项校验:
 *   1. me 本身合法
 *   2. me->ops 已填充
 *   3. 子类必填的 ops 函数指针 已填充
 *
 * 三层校验防止子类忘填纯虚函数, 或实例初始化路径出错使 ops 仅部分填充.
 * 见 ch15 § 15.2 父类层.
 */

#include "drivers/led/led_base.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops)
{
	if (!me || !name || !ops)
		return -1;

	me->ops   = ops;
	me->name  = name;
	me->is_on = false;
	return 0;
}

int led_on(struct led_base *me)
{
	if (!me)
		return -1;
	/* 必填: on 是 LED 的核心能力, 子类必须实现 (ch14 § 14.2). */
	assert(me->ops && me->ops->on &&
	       "led_on: subclass must implement on()");
	{
		int rc = me->ops->on(me);
		if (rc == 0)
			me->is_on = true;
		return rc;
	}
}

int led_off(struct led_base *me)
{
	if (!me)
		return -1;
	assert(me->ops && me->ops->off &&
	       "led_off: subclass must implement off()");
	{
		int rc = me->ops->off(me);
		if (rc == 0)
			me->is_on = false;
		return rc;
	}
}

int led_set_brightness(struct led_base *me, uint8_t level)
{
	if (!me || !me->ops)
		return -1;
	if (!me->ops->set_brightness) {
		/* 选填: 子类不支持调亮度, 父类走默认 no-op (ch14 § 14.3). */
		printf("  [%s] no dimming, skip (level=%u)\n",
		       me->name, (unsigned)level);
		return 0;
	}
	return me->ops->set_brightness(me, level);
}
