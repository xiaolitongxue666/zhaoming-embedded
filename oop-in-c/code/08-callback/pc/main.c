/* SPDX-License-Identifier: MIT */
/*
 * main.c - 函数指针当参数 + 回调注册
 *
 * 第一段: test_led(&red_led.base, gpio_on_pull_high, gpio_off);
 *         同一个 test_led 函数, 传不同 on/off 跑出不同行为.
 *
 * 第二段: 注册一个 on_state_change 回调, LED 开关时自动调一下.
 *         "把电话给朋友, 朋友帮你拨".
 */

#include <stdio.h>
#include "led.h"

/* 应用层注册的回调: 把 LED 状态变化打到日志 */
static void log_state_change(struct led_base *me, bool new_state)
{
	printf("    !! callback: \"%s\" became %s\n",
	       me->name,
	       new_state ? "ON" : "OFF");
}

int main(void)
{
	struct led_gpio red_led;
	struct led_gpio blue_led;

	printf("========================================\n");
	printf("  Function pointer as a parameter.\n");
	printf("  test_led / register_callback.\n");
	printf("========================================\n\n");

	led_gpio_init(&red_led, "red", 13);
	led_gpio_init(&blue_led, "blue", 15);

	printf("\n--- test_led(&red_led.base, pull_high, gpio_off) ---\n");
	test_led(&red_led.base, gpio_on_pull_high, gpio_off);

	printf("\n--- test_led(&blue_led.base, pull_high, gpio_off) ---\n");
	test_led(&blue_led.base, gpio_on_pull_high, gpio_off);

	printf("\n--- Register state callback for red_led ---\n");
	led_register_state_cb(&red_led.base, log_state_change);

	printf("\n--- Toggle red_led, callback should fire ---\n");
	led_on(&red_led.base);
	led_off(&red_led.base);

	printf("\n--- Unregister callback (pass NULL) ---\n");
	led_register_state_cb(&red_led.base, NULL);
	led_on(&red_led.base);
	led_off(&red_led.base);

	printf("\n========================================\n");
	printf("  test_led works for any LED.\n");
	printf("  Callbacks deliver events upward.\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
