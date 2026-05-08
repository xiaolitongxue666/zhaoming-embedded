/* SPDX-License-Identifier: MIT */
/**
 * @file  sensor_base.c
 * @brief sensor 父类层 -- 共有 init + 父类统一接口 (全必填·接口风格)
 *
 * @details
 * 父类做两件事:
 *   1) sensor_base_init - 共有字段 init (ops + name), 子类 init 第一行
 *      调一次. 风格跟 led_base_init 一字不变.
 *   2) sensor_read / sensor_calibrate / sensor_self_test - 父类统一接口,
 *      三个函数体都是一行 me->ops->xxx(me), 三件套全部走 assert 必填.
 *
 * 接口 (interface) = 全部必填的合同. read / calibrate / self_test 每一项
 * 都不能少. 一个 sensor 不能读, 或者不能校准, 或者不能自检, 它就不算
 * sensor.
 *
 * 三个统一接口函数全部 assert, 零容忍. 这是 C 模拟"纯接口"的形态,
 * 对应 C++ 全纯虚 class / Java interface.
 *
 * 见 ch14 § 14.4 (全必填·接口).
 */

#include "sensor_base.h"
#include <assert.h>
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

/* ============== 父类统一接口 (全必填) ============== */

int sensor_read(struct sensor_base *me, int32_t *out)
{
	if (!me || !out)
		return -1;

	assert(me->ops && me->ops->read &&
	       "sensor.read is part of the interface contract");
	return me->ops->read(me, out);
}

int sensor_calibrate(struct sensor_base *me)
{
	if (!me)
		return -1;

	assert(me->ops && me->ops->calibrate &&
	       "sensor.calibrate is part of the interface contract");
	return me->ops->calibrate(me);
}

int sensor_self_test(struct sensor_base *me)
{
	if (!me)
		return -1;

	assert(me->ops && me->ops->self_test &&
	       "sensor.self_test is part of the interface contract");
	return me->ops->self_test(me);
}
