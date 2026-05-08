/* SPDX-License-Identifier: MIT */
/**
 * @file  leds.h
 * @brief LED 模块对外暴露的全局句柄 - 应用层零硬件字样
 *
 * @details
 * 跟 ch12 一样的封装形态: 应用层只 include 这一个头文件, 看到的全是
 * struct led_base * 句柄 (extern 声明), 看不到 struct led_gpio /
 * struct led_pwm / struct led_i2c 任何子类. 硬件细节 (pin / channel /
 * addr) 全部锁在 led_board_init.c.
 *
 * ch13 主题是 container_of: GPIO 子类故意把 base 挪到第二个字段, 让
 * offsetof(struct led_gpio, base) != 0, 子类内部用 container_of(base,
 * struct led_gpio, base) 反推回子类指针. 这件事跟应用层无关 (应用层
 * 看到的还是 struct led_base * 句柄), 所以本章在 ch12 的封装基础上一字
 * 不退, 只是把 container_of 演示放进 led_board_init.c 的启动期日志 +
 * 子类 .c 内部.
 */

#ifndef LEDS_H
#define LEDS_H

#include "led_base.h"

extern struct led_base *g_led_error;
extern struct led_base *g_led_status;
extern struct led_base *g_led_network;

int led_board_init(void);

#endif /* LEDS_H */
