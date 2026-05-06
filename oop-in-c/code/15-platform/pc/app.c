/* SPDX-License-Identifier: MIT */
/*
 * app.c - 应用层
 *
 * 三个业务函数。整个文件里 grep gpio_write / pwm_ / i2c_ / HAL_ / sysfs /
 * platform_ops：全部 0 命中。应用层不认识硬件，硬件是谁它都不问，只
 * 通过 led_base * 句柄调 led_on / led_off / led_set_brightness。
 *
 * 这就是 ch15 想让你看见的：换主板（GPIO 换 PWM 换 I2C）、换平台（PC
 * 切 STM32 切 Linux）、换芯片（ch16 平台层下面再加一层），上面这三个
 * 业务函数都 0 改动。见 ch15 § 15.6 应用层。
 */

#include "leds.h"
#include <stdio.h>

void alarm_blink(void)
{
	printf("\n--- alarm_blink ---\n");
	led_on(g_led_error);
	led_off(g_led_error);
}

void status_breathe(void)
{
	printf("\n--- status_breathe ---\n");
	led_set_brightness(g_led_status, 30);
	led_set_brightness(g_led_status, 80);
	led_set_brightness(g_led_status,  0);
}

void power_on_test(void)
{
	printf("\n--- power_on_test ---\n");
	led_on(g_led_error);    led_off(g_led_error);
	led_on(g_led_status);   led_off(g_led_status);
	led_on(g_led_network);  led_off(g_led_network);
}
