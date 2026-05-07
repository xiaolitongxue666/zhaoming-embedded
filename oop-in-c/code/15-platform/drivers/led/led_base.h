/* SPDX-License-Identifier: MIT */
/*
 * led_base.h - LED 父类层 (drivers/led, ch15 跨 MCU 共享层).
 *
 * 这一层和 pc/ 教学版一字不差: 只有 led_base + ops 表 + 父类统一接口.
 * 区别只在于子类底下不再调 platform_gpio_xxx 模拟函数, 而是调
 * platform_pin_xxx / platform_pwm_xxx / platform_i2c_xxx ops 表层接口.
 *
 * 应用层只 #include "drivers/led/led_base.h" 拿到 struct led_base * 句柄,
 * 永远不直接 #include 子类头. 见 ch15 § 15.11.5 "STM32 vs NXP".
 */

#ifndef DRIVERS_LED_LED_BASE_H
#define DRIVERS_LED_LED_BASE_H

#include <stdbool.h>
#include <stdint.h>

struct led_base;

/* led_ops - LED 子类必须实现的虚函数表 (ch14 必填 + 选填混合形态).
 *   on / off          必填, 子类不填走父类 assert
 *   set_brightness    选填, 子类不填走父类默认 no-op
 */
struct led_ops {
	int (*on)(struct led_base *me);
	int (*off)(struct led_base *me);
	int (*set_brightness)(struct led_base *me, uint8_t level);
};

/* led_base - 所有 LED 子类的父类.
 *   ops    指向子类的虚函数表 (static const, 实例间共享)
 *   name   实例名, 调试 / 日志友好
 *   is_on  当前开关状态
 */
struct led_base {
	const struct led_ops *ops;
	const char           *name;
	bool                  is_on;
};

/* 公开接口 - 应用层只调这一组, 不直接访问 ops */
int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);
int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_set_brightness(struct led_base *me, uint8_t level);

#endif /* DRIVERS_LED_LED_BASE_H */
