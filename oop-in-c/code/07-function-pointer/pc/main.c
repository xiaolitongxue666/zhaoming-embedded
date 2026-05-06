/* SPDX-License-Identifier: MIT */
/*
 * main.c - 同一个 led_gpio_on, 不同的 on_func, 不同的行为
 *
 * 红灯填 gpio_on_pull_high  (拉高点亮)
 * 绿灯填 gpio_on_pull_low   (拉低点亮，低有效)
 *
 * 应用层都调 led_gpio_on(&xxx_led), 不管底下走哪一支.
 *
 * 还演示了"运行时换号码": 给红灯把 on_func 换成 pull_low,
 * 不重新 init, 行为立刻变.
 */

#include <stdio.h>
#include "led.h"

int main(void)
{
	struct led_gpio red_led;
	struct led_gpio green_led;

	printf("========================================\n");
	printf("  Function pointer field: swap behavior.\n");
	printf("  led_gpio_on() calls me->on_func(me).\n");
	printf("========================================\n\n");

	printf("--- Init red_led with pull_high on_func ---\n");
	led_gpio_init(&red_led, "red", 13, gpio_on_pull_high);

	printf("\n--- Init green_led with pull_low on_func ---\n");
	led_gpio_init(&green_led, "green", 14, gpio_on_pull_low);

	printf("\n--- led_gpio_on(&red_led) -> pull_high ---\n");
	led_gpio_on(&red_led);

	printf("\n--- led_gpio_on(&green_led) -> pull_low ---\n");
	led_gpio_on(&green_led);

	printf("\n--- swap red_led on_func at runtime to pull_low ---\n");
	red_led.on_func = gpio_on_pull_low;
	led_gpio_on(&red_led);

	printf("\n--- Cleanup ---\n");
	led_gpio_off(&red_led);
	led_gpio_off(&green_led);

	printf("\n========================================\n");
	printf("  led_gpio_on stays one line of code.\n");
	printf("  on_func field decides actual behavior.\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
