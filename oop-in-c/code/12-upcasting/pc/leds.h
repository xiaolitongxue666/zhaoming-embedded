/* SPDX-License-Identifier: MIT */
/**
 * @file  leds.h
 * @brief 板级对外暴露的全局 LED 句柄 - 应用层零硬件字样
 *
 * @details
 * 本章 (ch12 § 12.4 ~ § 12.6) 的工程化形态:
 *   - 应用层只 include 这一个头文件
 *   - 看到的全是 struct led_base * 句柄 (extern 声明), 看不到
 *     struct led_gpio / struct led_pwm / struct led_i2c 任何子类
 *   - 硬件细节 (pin / channel / addr) 全部锁在 board_init.c
 *   - 换硬件方案 (报警灯 GPIO -> PWM 调光), 应用层 0 改动,
 *     只改 board_init.c 的 3 行 (实例化、init、绑定)
 *
 * 这就是向上转型在工程里的全部威力: 应用层和硬件之间隔着一个
 * struct led_base * 句柄, 硬件那头怎么换, 句柄这头一律不知道.
 *
 * grep 验证 (在本目录跑):
 *   grep -n "led_gpio\|led_pwm\|led_i2c" main.c    # 0 行
 *   grep -n "gpio_write\|pwm_\|i2c_"      main.c   # 0 行
 */

#ifndef LEDS_H
#define LEDS_H

#include "led.h"

extern struct led_base *g_led_error;
extern struct led_base *g_led_status;
extern struct led_base *g_led_network;

int board_init(void);

#endif /* LEDS_H */
