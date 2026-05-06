/* SPDX-License-Identifier: MIT */
/*
 * leds.h - 板级层对外暴露的全局句柄
 *
 * 三个 struct led_base * 全局句柄 + board_init() 入口。注意句柄类型是
 * 父类 led_base *，不是具体子类——这是 ch12 向上转型在 ch15 工程里的
 * 落地。应用层拿到 g_led_error 也分不清它底层是 GPIO 还是 PWM 还是 I2C。
 *
 * 见 ch15 § 15.4 + ch12 向上转型。
 */
#ifndef LEDS_H
#define LEDS_H

#include "led.h"

extern struct led_base *g_led_error;
extern struct led_base *g_led_status;
extern struct led_base *g_led_network;

void board_init(void);

#endif
