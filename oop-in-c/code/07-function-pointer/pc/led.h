/* SPDX-License-Identifier: MIT */
/**
 * @file  led.h
 * @brief struct led_gpio 加一个函数指针字段 on_func - 解决"换硬件就要改源码"
 *
 * @details
 * 本章核心问题 (见 ch07 § 7.1 ~ § 7.5):
 *   ch01 ~ ch06 里 led_on() 函数体直接调 platform_gpio_write, "开灯怎么做"
 *   被写死在源码里. 加一种新的 LED (高有效 vs 低有效, 或换成 PWM) 就要
 *   改 led.c, 改完还可能影响所有已经在跑的 LED.
 *
 * 本章答案: 把"调谁"做成可填的字段, 函数地址也是数据.
 *   ch06 子类:
 *     struct led_gpio { struct led_base base; uint8_t pin; };
 *   ch07 子类 (本章):
 *     struct led_gpio {
 *         struct led_base base;
 *         uint8_t pin;
 *         int (*on_func)(struct led_gpio *me);    // <-- 新增, 存函数地址
 *     };
 *
 * on_func 这一行声明的读法: on_func 是一个变量, 它存的是"接受
 * struct led_gpio * 参数、返回 int 的函数"的地址. 函数名出现在
 * 表达式里时会自动退化成函数地址 (C99 § 6.3.2.1), 所以
 * me->on_func = gpio_on_pull_high; 就是把那段机器码的起点地址
 * 存进字段, me->on_func(me) 就是从字段取出地址跳过去执行.
 *
 * 工程意义 (见 ch07 § 7.5 "对扩展开放, 对修改关闭"):
 *   - 加新点亮策略: 写一个新的 gpio_on_xxx, 创建实例时填进 on_func
 *   - led_gpio_on() 这个函数从此一行不改 (对修改关闭)
 *   - 行为通过填新函数指针扩展 (对扩展开放)
 *
 * forward decl: on_func 的参数类型是 struct led_gpio *, 而它本身又
 * 是 struct led_gpio 的成员 -- 自引用类型. C 里靠前向声明绕开.
 */

#ifndef LED_H
#define LED_H

#include "led_base.h"

struct led_gpio;     /* forward decl, 因为 on_func 自己引用 struct led_gpio */

struct led_gpio {
	struct led_base base;
	uint8_t         pin;
	int (*on_func)(struct led_gpio *me);   /* 开灯怎么做 - 运行时定 */
};

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin,
                  int (*on_func)(struct led_gpio *me));
int led_gpio_on(struct led_gpio *me);
int led_gpio_off(struct led_gpio *me);

/* 两种现成的 on_func 实现 */
int gpio_on_pull_high(struct led_gpio *me);  /* 拉高点亮 */
int gpio_on_pull_low(struct led_gpio *me);   /* 拉低点亮 (低有效) */

#endif /* LED_H */
