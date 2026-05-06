/* SPDX-License-Identifier: MIT */
/*
 * main.c - 应用层
 *
 * 这个文件里没有 led_gpio、led_pwm、led_i2c 任何字样。
 * 它只用两个 struct led_base * 句柄。
 *
 * grep gpio_write、grep pwm_、grep i2c_，结果都是 0。
 * 应用层不认识硬件，硬件是谁它都不问。
 */

#include "leds.h"
#include <stdio.h>

static void alarm_blink(void)
{
	printf("\n--- alarm_blink ---\n");
	led_on(g_led_error);
	led_off(g_led_error);
}

static void network_heartbeat(void)
{
	printf("\n--- network_heartbeat ---\n");
	led_on(g_led_network);
	led_off(g_led_network);
}

int main(void)
{
	printf("=========================================\n");
	printf("  ch12 - upcasting\n");
	printf("  one led_base * handle, any subclass\n");
	printf("=========================================\n");

	board_init();

	alarm_blink();
	network_heartbeat();

	printf("\n=========================================\n");
	printf("  app layer: zero hardware reference\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
