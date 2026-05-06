/* SPDX-License-Identifier: MIT */
/*
 * main.c - 用 ops 表跑 test_led
 *
 * 同一个 test_led, 不同 ops 表, 跑出不同行为.
 * 一个参数 (ops 表指针) 搞定 3 个函数指针. 不会再因为顺序传反.
 */

#include <stdio.h>
#include "led.h"

int main(void)
{
	struct led_gpio red_led;
	struct led_pwm  blue_led;

	printf("========================================\n");
	printf("  ops table: pack action pointers as struct.\n");
	printf("  test_led takes one ops pointer.\n");
	printf("========================================\n\n");

	led_gpio_init(&red_led, "red", 13);
	led_pwm_init(&blue_led, "blue", 1, 70);

	printf("\n--- test_led(&red_led.base, &led_ops_gpio) ---\n");
	test_led(&red_led.base, &led_ops_gpio);

	printf("\n--- test_led(&blue_led.base, &led_ops_pwm) ---\n");
	test_led(&blue_led.base, &led_ops_pwm);

	printf("\n========================================\n");
	printf("  3 function pointers packed as 1 ops table.\n");
	printf("  Named access ops->on, ops->off (no order bug).\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
