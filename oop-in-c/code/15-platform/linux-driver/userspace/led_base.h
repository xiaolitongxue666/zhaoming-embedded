/* SPDX-License-Identifier: MIT */
/*
 * led_base.h - 父类层接口 + ops 表 (ch15 linux-driver/userspace 版).
 *
 * 这一档目录最关键的差别: **没有 platform 抽象层**. led_gpio.c 直接调
 * libgpiod, led_pwm.c 直接写 sysfs PWM 节点, led_i2c.c 直接 open /dev/i2c-N
 * + ioctl. Linux 内核已经把 GPIO / PWM / I2C 抽好成 driver model + 字符
 * 设备 + sysfs, 应用层再套一层 platform 就是过度封装.
 *
 * 这件事在 ch15 § 15.11 / § 15.15 / § 15.16 反复讲, 这里是代码兑现层.
 * 父类层 ops 表保持不变 -- OOP 抽象在所有平台都该有 (应用层句柄统一).
 *
 * 见 ch15 § 15.2 父类层 + § 15.11 你的代码在 Linux 上长什么样.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include <stdint.h>
#include <stdbool.h>

struct led_base;

struct led_ops {
	int (*on)(struct led_base *me);                 /* 必填 */
	int (*off)(struct led_base *me);                /* 必填 */
	int (*set_brightness)(struct led_base *me,      /* 选填 */
	                      uint8_t brightness);
};

struct led_base {
	const struct led_ops *ops;
	const char           *name;
	bool                  is_on;
};

int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);

/* 父类统一接口 */
int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_set_brightness(struct led_base *me, uint8_t brightness);

#endif /* LED_BASE_H */
