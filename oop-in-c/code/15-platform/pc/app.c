/* SPDX-License-Identifier: MIT */
/*
 * app.c - 应用层
 *
 * 三个真实业务函数:
 *   alarm_blink     报警闪烁    (报警灯亮 -> 灭)
 *   status_indicate 状态指示    (按错误码挑亮哪一盏)
 *   power_on_test   开机自检    (三盏灯依次亮一遍)
 *
 * grep 这一份文件 LedGpio / LedPwm / LedI2c / gpio_write / HAL_ / sysfs:
 * 全部 0 命中. 应用层不认识硬件, 硬件是谁它都不问, 只通过 led_base *
 * 句柄调 led_on / led_off / led_set_brightness.
 *
 * 板级 (board_init.c) 同一时刻挂着 GPIO+PWM+I2C 三种不同硬件的子类,
 * 应用层一个都不知道. 这就是"换硬件不改应用"在代码上的兑现.
 *
 * 见 ch15 § 15.5 应用层 + § 15.6 换硬件 diff.
 */

#include "leds.h"
#include "app.h"
#include <stdio.h>

void alarm_blink(void)
{
	printf("\n--- alarm_blink ---\n");
	led_on(g_led_error);
	led_off(g_led_error);
}

void status_indicate(int err_code)
{
	printf("\n--- status_indicate(err_code=%d) ---\n", err_code);
	if (err_code == 0)
		led_on(g_led_status);
	else
		led_on(g_led_error);
}

void power_on_test(void)
{
	printf("\n--- power_on_test ---\n");
	led_on(g_led_error);    led_off(g_led_error);
	led_on(g_led_status);   led_off(g_led_status);
	led_on(g_led_network);  led_off(g_led_network);
}
