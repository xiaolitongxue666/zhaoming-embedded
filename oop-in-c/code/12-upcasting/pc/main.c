/* SPDX-License-Identifier: MIT */
/*
 * main.c - 应用层
 *
 * 注意：这个文件里没有 led_gpio、led_pwm、led_i2c 任何字样。
 * 它只用三个 struct led_base * 句柄。
 *
 * grep gpio_write、grep pwm_、grep i2c_，结果都是 0。
 * 这就是向上转型在工程上的全部威力，硬件是谁，应用层不问。
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

static void status_breathe(void)
{
	printf("\n--- status_breathe ---\n");
	led_set_brightness(g_led_status, 30);
	led_set_brightness(g_led_status, 80);
	led_set_brightness(g_led_status,  0);
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
	status_breathe();

	printf("\n=========================================\n");
	printf("  app layer: zero hardware reference\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
