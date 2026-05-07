/* SPDX-License-Identifier: MIT */
/*
 * led_base.c - 父类层实现 (ch15 完整版, 风格 A)
 *
 * 两块内容:
 *   1) led_base_init - 共有 init, 子类 init 第一行调一次
 *   2) led_on / led_off / led_set_brightness - 父类统一接口,
 *      所有子类共用; 必填走 assert, 选填走父类默认行为
 *
 * 应用层只看到 led_on(handle) 这种调用, 不直接碰 ops 字段, 也不需要
 * 知道这颗 LED 是 GPIO 还是 PWM 还是 I2C.
 *
 * 见 ch15 § 15.2 父类层.
 */

#include "led_base.h"
#include "led.h"
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

/* ============== 父类统一接口 (必填 + 选填) ============== */

int led_on(struct led_base *me)
{
	if (!me)
		return -1;
	/* 必填: on 是 LED 的核心能力, 子类必须实现. 调试构建里 assert
	 * 抓到忘填的子类立刻 abort 给行号; Release 构建定义 NDEBUG 后
	 * assert 整行消失, 零运行时开销. 见 ch14 § 14.2. */
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
		/* 选填: GPIO / I2C 灯没有调光能力, 子类没填 set_brightness
		 * 就走父类的默认行为 (打印一行 "no dimming, skip"), 不崩.
		 * 见 ch14 § 14.3. */
		printf("  [%s] no dimming, skip (brightness=%u)\n",
		       me->name, (unsigned)brightness);
		return 0;
	}
	return me->ops->set_brightness(me, brightness);
}
