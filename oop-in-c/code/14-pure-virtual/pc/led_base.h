/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief 父类层公开头 - 字段集 + ops 表 + 共有 init + 父类统一接口 (ch14 版)
 *
 * @details
 * 父类层公开头, 跟 ch11 / ch12 / ch13 一脉相承的"集中点":
 *   - struct led_base 字段集 (ops + name + is_on)
 *   - struct led_ops  操作表字段集 (必填 on/off + 选填 set_brightness)
 *   - led_base_init   共有 init, 子类 init 第一行调一次
 *   - led_on/led_off/led_set_brightness  父类统一接口 (必填 + 选填混合策略)
 *
 * ch14 主线是"虚函数不实现 · 三种策略", 三个字段对应三种用法:
 *
 *     struct led_ops {
 *         int (*on)(struct led_base *me);                 // 必填
 *         int (*off)(struct led_base *me);                // 必填
 *         int (*set_brightness)(struct led_base *me,      // 选填
 *                               uint8_t brightness);
 *     };
 *
 * ops 表字段类型本身不变 (普通函数指针). "必填还是选填"这条纪律落在
 * 父类统一接口里 (见 led_base.c). 子类填了走子类, 没填: 必填的崩,
 * 选填的走默认. GPIO 子类故意只填 on/off, 不填 set_brightness, 演示
 * 选填策略. PWM 子类三件套全填.
 *
 * 子类实现里 (gpio_on / pwm_on / ...) 用 ch13 学的 container_of 反推
 * 子类指针, 强转那一招在 ch13 故意挪 base 演示后已经退役.
 *
 * 第三种策略 "全必填·接口" 由 sensor 这条独立 base 线演示 (sensor_base.h
 * + sensor_temp.h), 跟 led_base 不混.
 *
 * 见 ch14 § 14.2 (必填) / § 14.3 (选填) / § 14.4 (全必填·接口).
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

struct led_base;

/*
 * struct led_ops - 操作表.
 *
 * on / off 必填, set_brightness 选填. 三种字段类型完全一样, 区别只
 * 在父类统一接口里怎么处理 NULL.
 */
struct led_ops {
	int (*on)(struct led_base *me);                 /* 必填 */
	int (*off)(struct led_base *me);                /* 必填 */
	int (*set_brightness)(struct led_base *me,      /* 选填 */
	                      uint8_t brightness);
};

struct led_base {
	const struct led_ops *ops;     /* 第 0 字段 */
	const char           *name;
	bool                  is_on;
};

/*
 * led_base_init - 共有字段 init.
 *
 * 子类 init 第一行调这个函数, 把对应的 const ops 表作为常量传进来.
 * 共有字段 (ops / name / is_on) 一次填好.
 */
int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);

/* ============== 父类统一接口 (必填 + 选填) ==============
 *
 * 三个统一接口函数都接 base 指针. 函数体里走 me->ops->xxx(me), 那张
 * 表是跟着 me 自己跑的. 应用层不用知道下面挂的是哪种 LED.
 *
 * led_on / led_off    -> 必填 (assert)
 * led_set_brightness  -> 选填 (NULL 时父类默认行为)
 */
int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_set_brightness(struct led_base *me, uint8_t brightness);

#endif /* LED_BASE_H */
