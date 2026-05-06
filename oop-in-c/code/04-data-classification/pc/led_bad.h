/* SPDX-License-Identifier: MIT */
/*
 * led_bad.h - 反面教材：全局变量满天飞
 *
 * ch04 之前的"坏代码"。5 个全局变量散在 led_bad.c 文件开头，
 * 两个 LED 共享 g_pin，第二次 bad_led_init 覆盖第一次。
 *
 * 这个文件存在的唯一目的：让你看到全局变量有多危险。
 * 正确写法看 led.h / led.c。
 */

#ifndef LED_BAD_H
#define LED_BAD_H

#include "platform.h"

int bad_led_init(uint8_t pin);
int bad_led_on(void);
int bad_led_off(void);
int bad_led_set_brightness(uint8_t brightness);

/* 暴露 g_pin 让 main.c 能"看到 bug" */
int bad_led_get_pin(void);
int bad_led_get_brightness(void);

#endif /* LED_BAD_H */
