/* SPDX-License-Identifier: MIT */
/*
 * sensor.h - sensor 子类 + ops 表 + 父类统一接口 (全必填·接口风格演示)
 *
 * 接口风格的 ops 表三件套全填, 子类 ops 表里少一个, 父类统一接口里
 * assert 立刻爆.
 *
 *     struct sensor_ops {
 *         int (*read)(struct sensor_base *me, int32_t *out);   // 必填
 *         int (*calibrate)(struct sensor_base *me);            // 必填
 *         int (*self_test)(struct sensor_base *me);            // 必填
 *     };
 *
 * 子类一: temp_sensor (温度传感器), 三件套全填. main.c 里串行调
 * sensor_self_test / sensor_calibrate / sensor_read 演示完整接口.
 *
 * 见 ch14 § 14.4 / § 14.7.3.
 */

#ifndef SENSOR_H
#define SENSOR_H

#include "sensor_base.h"

/*
 * struct sensor_ops - 接口表.
 *
 * 三个字段全部必填. 字段类型本身和 led_ops 一样是普通函数指针, "全部
 * 必填"这条纪律落在父类统一接口的 assert 里 (见 sensor.c).
 */
struct sensor_ops {
	int (*read)(struct sensor_base *me, int32_t *out);
	int (*calibrate)(struct sensor_base *me);
	int (*self_test)(struct sensor_base *me);
};

/* 子类一: 温度传感器 */
struct temp_sensor {
	struct sensor_base base;
	int32_t            last_value;
};

int temp_sensor_init(struct temp_sensor *me, const char *name);

/* ============== 父类统一接口 (全必填·接口) ==============
 *
 * 三个统一接口函数都走 assert 必填. 子类 ops 表少一个, 调试期立刻爆.
 * 这就是 C 模拟"接口"的形态.
 */
int sensor_read(struct sensor_base *me, int32_t *out);
int sensor_calibrate(struct sensor_base *me);
int sensor_self_test(struct sensor_base *me);

#endif /* SENSOR_H */
