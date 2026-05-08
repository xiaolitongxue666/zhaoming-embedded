/* SPDX-License-Identifier: MIT */
/*
 * sensor_temp.c - 温度传感器子类实现 (ch14 接口风格 demo)
 *
 * 接口风格的 ops 表三件套全填. 跟 led_gpio.c / led_pwm.c 一脉相承的
 * 子类实现风格: temp_xxx 静态函数 + static const temp_ops 表 +
 * temp_sensor_init 第一行调 sensor_base_init 把 ops 表交给父类.
 *
 * 子类实现里用 ch13 学的 container_of 反推子类指针, 跟 LED 体系一致.
 *
 * 见 ch14 § 14.4 (全必填·接口).
 */

#include "sensor_temp.h"
#include "container_of.h"
#include <stdio.h>

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
