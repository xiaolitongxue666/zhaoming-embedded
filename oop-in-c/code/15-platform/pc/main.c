/* SPDX-License-Identifier: MIT */
/*
 * main.c - ch15 完整 LED 框架演示
 *
 * 整本书 ch01 - ch14 一路学的所有 OOP 武器, 在这里组装成一份完整工程:
 *
 *   父类层 (led.h / led.c)        : led_base + ops 表 + 必填 / 选填
 *   子类层 (led.c)                : led_gpio / led_pwm / led_i2c, container_of 反推
 *   板级层 (leds.h / board_init.c): 三种子类混搭, 向上转型挂全局句柄
 *   应用层 (app.h / app.c)        : alarm_blink / status_indicate / power_on_test
 *
 * 主线: grep app.c 拿不到任何硬件字样 (LedGpio / LedPwm / LedI2c / gpio_write
 * 全部 0 命中). 应用层只用 led_base * 句柄. 换硬件方案 -> 改 board_init.c
 * 三行, app.c 0 改动. 这就是 ch15 章末金句:
 *
 *   好的架构不是让你写更多代码, 是让你改更少代码.
 *
 * 见 ch15 § 15.5 应用层 + § 15.6 换硬件 diff.
 */

#include "app.h"
#include "leds.h"
#include <stdio.h>

int main(void)
{
	printf("=========================================\n");
	printf("  ch15 - OOP complete framework demo\n");
	printf("=========================================\n");

	board_init();

	power_on_test();
	alarm_blink();
	status_indicate(0);   /* 正常 -> 状态灯 */
	status_indicate(1);   /* 故障 -> 报警灯 */

	printf("\n=========================================\n");
	printf("  app.c never named any hardware type\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
