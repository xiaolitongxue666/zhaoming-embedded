/* SPDX-License-Identifier: MIT */
/**
 * @file  sensor_base.h
 * @brief sensor_base 字段集 + 共有 init (接口风格父类)
 *
 * @details
 * 这一章在 LED 系列之外, 单独演示一种"全必填" ops 表的形态. 接口风格
 * (interface) 是把 ops 表所有字段都设成必填, 父类统一接口里每一个都
 * 走 assert.
 *
 * sensor_base 自身的字段集和 led_base 几乎一样: 一颗 const ops 指针打头,
 * 加一个名字字段. 区别只在父类统一接口怎么处理 NULL (见 sensor.c).
 *
 *     struct sensor_base {
 *         const struct sensor_ops *ops;     // 第 0 字段
 *         const char              *name;
 *     };
 *
 * 子类 init (temp_sensor_init / ...) 第一行调 sensor_base_init, 把对应
 * 的 const ops 表作为常量传进来, 一次填好. 风格和 led_base_init 一致.
 *
 * 见 ch14 § 14.4 (全必填·接口) / § 14.7.3 (什么时候选哪种).
 */

#ifndef SENSOR_BASE_H
#define SENSOR_BASE_H

#include <stdint.h>

/* 前向声明 - sensor_ops 完整定义在 sensor.h */
struct sensor_ops;

struct sensor_base {
	const struct sensor_ops *ops;     /* 第 0 字段 */
	const char              *name;
};

/*
 * sensor_base_init - 共有字段 init.
 *
 * 子类 init 第一行调这个函数, 把对应的 const ops 表作为常量传进来.
 */
int sensor_base_init(struct sensor_base *me, const char *name,
                     const struct sensor_ops *ops);

#endif /* SENSOR_BASE_H */
