/* SPDX-License-Identifier: MIT */
/**
 * @file  sensor_base.h
 * @brief sensor 父类层公开头 - 字段集 + ops 表 + 共有 init + 父类统一接口
 *        (接口风格 · 全必填)
 *
 * @details
 * 这一章在 LED 系列之外, 单独演示一种"全必填" ops 表的形态. 接口风格
 * (interface) 是把 ops 表所有字段都设成必填, 父类统一接口里每一个都
 * 走 assert.
 *
 * sensor_base 自身的字段集和 led_base 几乎一样: 一颗 const ops 指针打头,
 * 加一个名字字段. 区别只在父类统一接口怎么处理 NULL (见 sensor_base.c
 * 全 assert).
 *
 *     struct sensor_base {
 *         const struct sensor_ops *ops;     // 第 0 字段
 *         const char              *name;
 *     };
 *
 *     struct sensor_ops {
 *         int (*read)(struct sensor_base *me, int32_t *out);   // 必填
 *         int (*calibrate)(struct sensor_base *me);            // 必填
 *         int (*self_test)(struct sensor_base *me);            // 必填
 *     };
 *
 * 三个字段全部必填. 字段类型本身和 led_ops 一样是普通函数指针, "全部
 * 必填"这条纪律落在父类统一接口的 assert 里 (见 sensor_base.c).
 *
 * 子类 (sensor_temp) 头文件 sensor_temp.h 单独一份, 风格和 led_gpio.h
 * 一致 -- 父类公开接口集中在 sensor_base.h, 子类各自一对 .h / .c.
 *
 * 见 ch14 § 14.4 (全必填·接口) / § 14.7.3 (什么时候选哪种).
 */

#ifndef SENSOR_BASE_H
#define SENSOR_BASE_H

#include <stdint.h>

struct sensor_base;

/*
 * struct sensor_ops - 接口表.
 *
 * 三个字段全部必填. 字段类型本身和 led_ops 一样是普通函数指针, "全部
 * 必填"这条纪律落在父类统一接口的 assert 里 (见 sensor_base.c).
 */
struct sensor_ops {
	int (*read)(struct sensor_base *me, int32_t *out);
	int (*calibrate)(struct sensor_base *me);
	int (*self_test)(struct sensor_base *me);
};

struct sensor_base {
	const struct sensor_ops *ops;     /* 第 0 字段 */
	const char              *name;
};

/*
 * sensor_base_init - 共有字段 init.
 *
 * 子类 init 第一行调这个函数, 把对应的 const ops 表作为常量传进来.
 * 风格和 led_base_init 一致.
 */
int sensor_base_init(struct sensor_base *me, const char *name,
                     const struct sensor_ops *ops);

/* ============== 父类统一接口 (全必填·接口) ==============
 *
 * 三个统一接口函数全部走 assert 必填. 子类 ops 表少一个, 调试期立刻爆.
 * 这就是 C 模拟"接口"的形态, 对应 C++ 全纯虚 class / Java interface.
 */
int sensor_read(struct sensor_base *me, int32_t *out);
int sensor_calibrate(struct sensor_base *me);
int sensor_self_test(struct sensor_base *me);

#endif /* SENSOR_BASE_H */
