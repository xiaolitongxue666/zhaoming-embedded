/* SPDX-License-Identifier: MIT */
/*
 * sensor_temp.h - 温度传感器子类 (ch14 接口风格 demo)
 *
 * 子类 .h 只装两样东西: struct temp_sensor 字段集 (base 在第 0 字段) +
 * 构造函数 temp_sensor_init 声明. ops 表 (temp_read / temp_calibrate /
 * temp_self_test + temp_ops 表) static 锁在 sensor_temp.c 内, 应用层
 * 永远碰不到 temp_xxx 这一层.
 *
 * 接口风格的 ops 表三件套全填. 子类 ops 表少一个, 父类统一接口里
 * (sensor_base.c 的 sensor_read / sensor_calibrate / sensor_self_test)
 * assert 立刻爆.
 *
 * 风格和 led_gpio.h / led_pwm.h 一致 -- 父类公开接口集中在 sensor_base.h,
 * 子类各自一对独立的 .h / .c.
 *
 * 见 ch14 § 14.4 / § 14.7.3.
 */

#ifndef SENSOR_TEMP_H
#define SENSOR_TEMP_H

#include "sensor_base.h"

/*
 * 温度传感器子类: base 在第 0 字段, last_value 缓存最近一次读到的值.
 *
 * 真实工业温度传感器 (PT100 / TMP102 / BME280 ...) 字段集会更复杂
 * (校准系数 / 总线句柄 / 缓存策略), 这里收缩到一个 last_value 让
 * 主线聚焦"接口风格"机制本身.
 */
struct temp_sensor {
	struct sensor_base base;       /* 父类, 第 0 字段 */
	int32_t            last_value;
};

int temp_sensor_init(struct temp_sensor *me, const char *name);

#endif /* SENSOR_TEMP_H */
