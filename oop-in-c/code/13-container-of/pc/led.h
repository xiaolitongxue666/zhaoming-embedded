/* SPDX-License-Identifier: MIT */
/*
 * led.h - led_ops 字段集 + 父类统一接口 (ch13 版)
 *
 * 这一份 led.h 在 ch13 退到"公开接口集中点": 应用层 #include 这一个
 * 头就拿到 struct led_ops + led_on / led_off / led_set_brightness +
 * struct led_base. 子类头文件 (led_gpio.h / led_pwm.h / led_i2c.h)
 * 各自单独一份, 只在认识硬件的位置 (main.c / board_init.c) include.
 *
 * led_ops 字段集比 ch12 多一个 set_brightness, PWM 子类填了, GPIO 和
 * I2C 子类没填. 这一章主线是 container_of, set_brightness 字段只是
 * 顺便挂上, 跨章演化更平滑. "字段没填怎么办" 留给 ch14 主题展开.
 *
 * 见 ch13 § 13.6 在 gpio_on 里用一下 + § 13.8.2 base 想放哪就放哪
 *   + § 13.8.5 配套代码 ops 表多了一个字段.
 */

#ifndef LED_H
#define LED_H

#include "led_base.h"
#include <stdint.h>
#include <stdbool.h>

/*
 * struct led_ops - 操作表 (ops 表).
 *
 * 比 ch12 多一个 set_brightness 字段. PWM 子类填了 pwm_set_brightness,
 * GPIO / I2C 子类不填, 字段保持 NULL. led_set_brightness 默认行为是
 * "字段为 NULL 安静返回 0", 跟 ch14 三种策略里"安静默认"那一种对应,
 * 但 ch14 才正式展开"虚函数不实现"的机制讨论.
 */
struct led_ops {
	int (*on)(struct led_base *me);
	int (*off)(struct led_base *me);
	int (*set_brightness)(struct led_base *me, uint8_t brightness);
};

/* 父类统一接口 - 实现在 led_base.c (调度方有限错误检查 + 走 ops 表) */
int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_set_brightness(struct led_base *me, uint8_t brightness);

#endif /* LED_H */
