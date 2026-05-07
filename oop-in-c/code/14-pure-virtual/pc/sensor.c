/* SPDX-License-Identifier: MIT */
/*
 * sensor.c - 父类统一接口 (全必填·接口) + temp_sensor 子类
 *
 * 接口 (interface) = 全部必填的合同. read / calibrate / self_test 每一项
 * 都不能少. 一个 sensor 不能读, 或者不能校准, 或者不能自检, 它就不算
 * sensor.
 *
 * 三个统一接口函数全部 assert, 零容忍. 这是 C 模拟"纯接口"的形态,
 * 对应 C++ 全纯虚 class / Java interface.
 *
 * 子类 init (temp_sensor_init) 第一行调 sensor_base_init, 把对应的
 * const ops 表作为常量传进来. 风格和 led_gpio_init / led_pwm_init 一致,
 * 就是把"父类通用 init 接 ops"这套机制套到 sensor 体系上.
 *
 * 见 ch14 § 14.4 (全必填·接口).
 */

#include "sensor.h"
#include "container_of.h"
#include <assert.h>
#include <stdio.h>

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

/* ============== temp_sensor 子类: 三件套全填 ============== */

static int temp_read(struct sensor_base *me, int32_t *out)
{
	struct temp_sensor *self = container_of(me, struct temp_sensor, base);
	self->last_value = 25;       /* 假装从硬件读了一个温度 */
	*out = self->last_value;
	printf("  [%s] read = %d C\n", me->name, *out);
	return 0;
}

static int temp_calibrate(struct sensor_base *me)
{
	printf("  [%s] calibrate\n", me->name);
	return 0;
}

static int temp_self_test(struct sensor_base *me)
{
	printf("  [%s] self_test\n", me->name);
	return 0;
}

/*
 * 子类 ops 表: 三件套全填. 任何一项不填, 父类统一接口的 assert 立刻爆.
 * 这就是接口风格 -- 合同里每一项都不能少.
 */
static const struct sensor_ops temp_ops = {
	.read      = temp_read,
	.calibrate = temp_calibrate,
	.self_test = temp_self_test,
};

int temp_sensor_init(struct temp_sensor *me, const char *name)
{
	int rc;
	if (!me)
		return -1;

	rc = sensor_base_init(&me->base, name, &temp_ops);
	if (rc != 0)
		return rc;

	me->last_value = 0;
	return 0;
}
