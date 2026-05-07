/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.c
 * @brief 父类层 -- 共有 init + 父类统一接口 led_on / led_off / led_set_brightness (ch13 版)
 *
 * @details
 * 父类做两件事:
 *   1) led_base_init - 共有字段 init (ops / name / is_on), 子类 init
 *      第一行调一次. 跟 ch10 / ch11 一字不动.
 *   2) led_on / led_off / led_set_brightness - 父类统一接口, 函数体
 *      一行 me->ops->xxx(me), 三种子类共用.
 *
 * led_on / led_off 用 assert 抓"忘填 on / off"的子类: 调试构建里 abort
 * 把低级错暴露在调试期, Release 构建定义 NDEBUG, assert 整行消失,
 * 0 运行时开销. 这是 C 模拟"必填"的等价物 (跟 ch14 的"必填策略"对应).
 *
 * led_set_brightness 是选填 -- 子类 ops 没填, 父类的统一接口安静返回 0
 * (这一章先用最简默认, ch14 把"安静默认"展开成完整的选填策略).
 *
 * 这一章 (ch13) 所有 container_of 相关变化在子类实现层 (led_gpio.c
 * 等) 里, 父类层一字不变.
 */

#include "led.h"
#include <assert.h>
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

int led_on(struct led_base *me)
{
	if (!me)
		return -1;
	/* on 是 LED 的核心能力, 子类必须实现. 调试构建里 assert 抓到
	 * 忘填的子类立刻 abort, Release 构建定义 NDEBUG 后 assert 整行
	 * 消失, 0 运行时开销. */
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
	/* set_brightness 是选填字段, 子类没实现就走父类默认行为
	 * (这里默认 = 安静返回 0). */
	if (!me->ops->set_brightness)
		return 0;
	return me->ops->set_brightness(me, brightness);
}
