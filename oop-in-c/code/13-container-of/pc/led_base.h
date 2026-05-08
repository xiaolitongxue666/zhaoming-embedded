/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief 父类层公开头 - 字段集 + ops 表 + 共有 init + 父类统一接口 (ch13 版)
 *
 * @details
 * 父类层公开头, 跟 ch11 / ch12 一脉相承的"集中点":
 *   - struct led_base 字段集 (ops + name + is_on)
 *   - struct led_ops  操作表字段集 (ch13 比 ch12 多一个 set_brightness 槽位)
 *   - led_base_init   共有 init, 子类 init 第一行调一次
 *   - led_on/led_off/led_set_brightness  父类统一接口
 *
 * ch13 主题是 container_of, 字段集本身没动. ops 仍然是 led_base 的第一个
 * 字段, 这一条是 led_base 自身的 ABI 不变量, 不被 ch13 的"教学变形"影响:
 *
 *     struct led_base {
 *         const struct led_ops *ops;     // 第 0 字段 (base 自身视角)
 *         const char           *name;
 *         bool                  is_on;
 *     };
 *
 * 注意一个区别:
 *   - "ops 在 led_base 的第 0 字段" -- 永远成立 (base 自身视角)
 *   - "led_base 在 led_gpio 的第 0 字段" -- ch13 故意打破 (容器视角)
 * 第二条原本撑住了 ch12 那一招 (struct led_gpio *)me 强转, 一旦容器里
 * base 不在第 0 偏移, 强转就会算错地址. 这正是 ch13 要解的问题, 也是
 * container_of 的核心威力.
 *
 * ops 表新增的 set_brightness 字段在 PWM 子类里填了, GPIO 和 I2C 子类
 * 没填, 父类统一接口 led_set_brightness 默认行为是"字段为 NULL 安静
 * 返回 0", 跟 ch14 § 14.3 的选填策略对应. ch14 才正式展开"虚函数不
 * 实现"的机制讨论.
 *
 * 见 ch13 § 13.3 强转能用但脆弱 + § 13.6 在 gpio_on 里用一下 +
 *   § 13.8.2 base 想放哪就放哪 + § 13.8.5 配套代码 ops 表多了一个字段.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

struct led_base;

/*
 * struct led_ops - 操作表 (ops 表).
 *
 * 比 ch12 多一个 set_brightness 字段. PWM 子类填了 pwm_set_brightness,
 * GPIO / I2C 子类不填, 字段保持 NULL. led_set_brightness 默认行为是
 * "字段为 NULL 安静返回 0", 跟 ch14 § 14.3 的选填策略对应, ch14 才
 * 正式展开"虚函数不实现"的机制讨论.
 */
struct led_ops {
	int (*on)(struct led_base *me);
	int (*off)(struct led_base *me);
	int (*set_brightness)(struct led_base *me, uint8_t brightness);
};

struct led_base {
	const struct led_ops *ops;     /* 第 0 字段 (base 自身视角) */
	const char           *name;
	bool                  is_on;
};

/*
 * led_base_init - 共有字段 init.
 *
 * 子类 init (led_gpio_init / led_pwm_init / led_i2c_init) 第一行调
 * led_base_init, 把"我用哪张 ops 表"作为常量传进来. 一次填好,
 * 之后对象不再碰 ops / name / is_on.
 */
int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);

/* 父类统一接口 - 实现在 led_base.c (调度方有限错误检查 + 走 ops 表) */
int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_set_brightness(struct led_base *me, uint8_t brightness);

#endif /* LED_BASE_H */
