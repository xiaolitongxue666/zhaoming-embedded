/* SPDX-License-Identifier: MIT */
/*
 * main.c - 每颗 LED 自带 ops, 应用层只调 led_xxx(&me.base)
 *
 * 一个 led_on, 内部 me->ops->on(me), 自动 dispatch 到 gpio/pwm 实现.
 */

#include <stdio.h>
#include "led.h"

int main(void)
{
	struct led_gpio red_led;
	struct led_pwm  blue_led;

	printf("========================================\n");
	printf("  vptr lands inside led_base.\n");
	printf("  led_on(&me.base) -> me->ops->on(me).\n");
	printf("========================================\n\n");

	led_gpio_init(&red_led, "red", 13);
	led_pwm_init(&blue_led, "blue", 1, 70);

	printf("\n--- led_on(&red_led.base) -> dispatch to gpio ---\n");
	led_on(&red_led.base);

	printf("\n--- led_on(&blue_led.base) -> dispatch to pwm ---\n");
	led_on(&blue_led.base);

	printf("\n--- led_toggle(&red_led.base) ---\n");
	led_toggle(&red_led.base);

	printf("\n--- led_toggle(&blue_led.base) ---\n");
	led_toggle(&blue_led.base);

	printf("\n========================================\n");
	printf("  Each LED carries its own ops table pointer.\n");
	printf("  No need to pass ops at every call.\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
