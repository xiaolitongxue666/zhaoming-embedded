/* SPDX-License-Identifier: MIT */
/**
 * @file  leds.h
 * @brief LED 模块对外暴露的全局句柄 - 应用层零硬件字样
 *
 * @details
 * 跟 ch12 / ch13 一样的封装形态: 应用层只 include 这一个头文件, 看到的
 * 全是 struct led_base * 句柄. 硬件细节锁在 led_board_init.c.
 *
 * ch14 主题三种 ops 策略 (必填 / 选填 / 接口) 是子类内部 + 父类统一
 * 接口的事, 跟应用层封装层无关. 所以本章在 ch12 的封装基础上一字不退.
 *
 * 本章只演示两盏 LED:
 *   - g_led_error  (GPIO 灯, 没填 set_brightness, 走父类默认行为)
 *   - g_led_status (PWM  灯, 全填, 调用走子类实现)
 * sensor 在 sensors.h 里另外暴露.
 */

#ifndef LEDS_H
#define LEDS_H

#include "led_base.h"

extern struct led_base *g_led_error;
extern struct led_base *g_led_status;

int led_board_init(void);

#endif /* LEDS_H */
