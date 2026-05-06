/* SPDX-License-Identifier: MIT */
/**
 * @file  led.h
 * @brief 函数指针当参数传 (test_led) + 回调注册 (led_register_state_cb)
 *
 * @details
 * 本章核心: 函数指针除了挂在字段上 (ch07), 还能当参数传给别人.
 *
 * 1) test_led(me, on, off) - 见 ch08 § 8.2:
 *    test_led 自己不写死调谁的 on/off. 调用方传函数指针进来, 让
 *    test_led 在内部"代为拨号". 视频金句: 函数指针的本质是延迟决定,
 *    不是现在就定死, 而是到时候再说.
 *
 *    关键好处: 测试时想临时换一个调试版的 on (比如先打 log 再开灯),
 *    不用 mutate 实例的字段. 字段污染了下次正常调用就乱套, 直接
 *    传参不留痕.
 *
 * 2) led_register_state_cb (在 led_base.h 声明) - 见 ch08 § 8.7:
 *    应用层把自己的函数地址挂到 led_base.on_state_change 字段上,
 *    led_on / led_off 状态翻转时反过来调一次. 这是工业代码里"驱动
 *    通知应用层"的标准做法 -- driver 不 #include 应用层头文件,
 *    靠预留回调字段解耦.
 *
 * 子类 struct led_gpio 这一章只有 base + pin 两个字段.
 * (ch07 演示了独立函数指针变量, 没有把指针塞进 struct;
 *  本章把函数指针当参数传, 是函数指针的下一种用法.)
 * ops 系统化要等 ch09 引入 struct led_ops.
 */

#ifndef LED_H
#define LED_H

#include "led_base.h"

struct led_gpio {
	struct led_base base;
	uint8_t         pin;
};

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin);

/* 给 led_base 用的 on/off 实现 (走 GPIO 拉电平) */
int gpio_on_pull_high(struct led_base *me);
int gpio_off(struct led_base *me);

/*
 * 通用测试函数: 不写死调谁的 on/off, 调用方传进来.
 * 这是函数指针当参数最直接的形态.
 */
int test_led(struct led_base *me,
             int (*on)(struct led_base *me),
             int (*off)(struct led_base *me));

/* 调用方便包装: 通过 base 的 on_state_change 回调通知应用层 */
int led_on(struct led_base *me);
int led_off(struct led_base *me);

#endif /* LED_H */
