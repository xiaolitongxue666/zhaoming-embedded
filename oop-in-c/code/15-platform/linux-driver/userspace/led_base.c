/* SPDX-License-Identifier: MIT */
/*
 * led_base.c - 父类层实现 (ch15 linux-driver/userspace 版).
 *
 * 父类层和 drivers/led/led_base.c 在结构上完全一致 -- OOP 抽象不挑平台.
 * 唯一差别: Linux 用户态有 stdio, set_brightness 选填走 no-op 时打印
 * 一行调试信息 (跟 pc/ 一致); MCU 裸机版也是同款实现.
 *
 * 见 ch15 § 15.2 父类层.
 */

#include "led_base.h"
#include <assert.h>
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
	assert(me->ops && me->ops->on &&
	       "led_on: subclass must implement on()");
	return me->ops->on(me);
}

int led_off(struct led_base *me)
{
	if (!me)
		return -1;
	assert(me->ops && me->ops->off &&
	       "led_off: subclass must implement off()");
	return me->ops->off(me);
}

int led_set_brightness(struct led_base *me, uint8_t brightness)
{
	if (!me || !me->ops)
		return -1;
	if (!me->ops->set_brightness) {
		printf("  [%s] no dimming, skip (brightness=%u)\n",
		       me->name, (unsigned)brightness);
		return 0;
	}
	return me->ops->set_brightness(me, brightness);
}
