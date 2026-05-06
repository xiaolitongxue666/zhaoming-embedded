/* SPDX-License-Identifier: MIT */
/*
 * main.c - 多态完整图景
 *
 * 三件事完整版:
 *   1. ops 表 (ch09 建好)
 *   2. 每个对象自带 ops (ch10 落地)
 *   3. 调用时 me->ops->on(me) dispatch (本章)
 *
 * 加上 platform 层从函数式 (ch01) 重构成 ops 表式 (ch11).
 * 驱动层调 platform_gpio_xxx 封装函数, 跟 ch01 起一字不变. platform
 * 层内部走 ops dispatch, 应用层 / 驱动层一字不知.
 *
 * 演示一个 led_base * 数组循环跑 led_on, 同一行代码自动调到不同 ops.
 */

#include <stdio.h>
#include "led.h"
#include "platform_ops.h"

int main(void)
{
	struct led_gpio red_led;
	struct led_pwm  blue_led;
	struct led_gpio green_led;

	struct led_base *all_leds[3];

	printf("========================================\n");
	printf("  Polymorphism complete picture.\n");
	printf("  Same code, different behavior per LED.\n");
	printf("========================================\n\n");

	/* 第一步: 选 platform. 驱动层 / 应用层一字不知 platform 是谁. */
	platform_select(&platform_pc);
	putchar('\n');

	led_gpio_init(&red_led, "red", 13);
	led_pwm_init(&blue_led, "blue", 1, 70);
	led_gpio_init(&green_led, "green", 17);

	all_leds[0] = &red_led.base;
	all_leds[1] = &blue_led.base;
	all_leds[2] = &green_led.base;

	printf("\n--- Loop over led_base * array, call led_on ---\n");
	for (int i = 0; i < 3; ++i) {
		printf(" idx=%d:\n", i);
		led_on(all_leds[i]);
	}

	printf("\n--- Loop again, call led_off ---\n");
	for (int i = 0; i < 3; ++i)
		led_off(all_leds[i]);

	printf("\n========================================\n");
	printf("  Same led_on(...) line dispatched to:\n");
	printf("    [0] gpio_on  (red)\n");
	printf("    [1] pwm_on   (blue, duty=70)\n");
	printf("    [2] gpio_on  (green)\n");
	printf("  Pure runtime dispatch via ops table.\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
