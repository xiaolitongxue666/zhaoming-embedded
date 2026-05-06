/* SPDX-License-Identifier: MIT */
/**
 * @file  led.h
 * @brief 子类 + ops 表; 每颗 LED 自带 ops, 调用方不再传表
 *
 * @details
 * 上一章 (ch09) 的 test_led 还要调用方自己传 ops:
 *     test_led(&red_led.base, &led_ops_gpio);
 *
 * 这一章 led_base 已经带 ops 字段了 (见 led_base.h). 子类 init 把
 * 对应的 const ops 表作为常量传给 base init, 一次填好.
 * 调用形态变成:
 *
 *     led_gpio_init(&red_led, "red", 13);    // 子类 init 把 &led_ops_gpio
 *                                            //   作为常量传给 base init
 *     test_led(&red_led.base);               // 应用层只看见 base 指针
 *                                            //   内部走 me->ops->on(me)
 *
 * 子类 led_gpio / led_pwm 还是 base + 硬件参数, ops 不在子类里.
 */

#ifndef LED_H
#define LED_H

#include "led_base.h"

/*
 * led_action_fn - 用 typedef 给函数指针类型起短名.
 * "int (*)(struct led_base *)" 写一次还行, ops 表里写三次就累.
 */
typedef int (*led_action_fn)(struct led_base *me);

/*
 * struct led_ops - 操作表.
 *
 * 把同一种 LED 的所有"可变行为"打包. 一种 LED 实现填一张表,
 * 调用方按名字 ops->on / ops->off 访问, 不会传反.
 */
struct led_ops {
	led_action_fn on;
	led_action_fn off;
	led_action_fn toggle;
};

/* GPIO 子类: pin 在子类里 */
struct led_gpio {
	struct led_base base;
	uint8_t         pin;
};

/* PWM 子类: channel + duty 在子类里 */
struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin);
int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty);

/*
 * test_led - 通用测试函数, 一个参数 (base 指针) 搞定.
 *
 * 函数体内部直接 me->ops->on(me) / me->ops->off(me) /
 * me->ops->toggle(me). 调用方完全不用知道 me 下面挂的是哪张 ops 表 --
 * 那张表跟着 me 自己跑.
 */
int test_led(struct led_base *me);

/*
 * 两张现成的 ops 表 - extern 声明, 定义在 led.c.
 *
 * const + extern 的组合让这两张表落进 .rodata 段:
 *   - MCU 上烧到 Flash, 完全不占 RAM
 *   - 100 颗同类型 LED 共享同一张 12 字节的表
 *   - 链接期不可改, 防止运行时把 .on 字段意外改成野指针
 */
extern const struct led_ops led_ops_gpio;
extern const struct led_ops led_ops_pwm;

#endif /* LED_H */
