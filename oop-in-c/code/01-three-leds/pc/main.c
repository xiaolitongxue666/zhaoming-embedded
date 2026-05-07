/* SPDX-License-Identifier: MIT */
/*
 * main.c - "三颗 LED 三份代码" 的最朴素治法
 *
 * 三颗 LED 共用同一份 led_on / led_off / led_toggle 代码。
 * 区别只在传进去的 me 指针指向哪一张挂号单。
 *
 * 想加第 4、第 5、第 100 颗 LED?
 *   多开一个 struct led 实例就行 —— led_on 这个函数一行不用动。
 */

#include <stdio.h>
#include "led.h"

int main(void)
{
	struct led red_led;     /* 红灯 - PA.13 */
	struct led green_led;   /* 绿灯 - PA.14 */
	struct led blue_led;    /* 蓝灯 - PA.15 */

	printf("========================================\n");
	printf("  Three LEDs, one set of code.\n");
	printf("  me pointer decides who to operate.\n");
	printf("========================================\n\n");

	printf("--- Init ---\n");
	led_init(&red_led,   PIN_NUM('A', 13));   /* 0x0D = PA.13 */
	led_init(&green_led, PIN_NUM('A', 14));   /* 0x0E = PA.14 */
	led_init(&blue_led,  PIN_NUM('A', 15));   /* 0x0F = PA.15 */

	printf("\n--- Turn on RED ---\n");
	led_on(&red_led);

	printf("\n--- Turn on GREEN ---\n");
	led_on(&green_led);

	printf("\n--- Turn on BLUE ---\n");
	led_on(&blue_led);

	printf("\n--- Turn off RED ---\n");
	led_off(&red_led);

	printf("\n--- Toggle GREEN ---\n");
	led_toggle(&green_led);

	printf("\n--- Set BLUE brightness to 75%% ---\n");
	led_set_brightness(&blue_led, 75);

	printf("\n--- Set BLUE brightness to 0%% (auto off) ---\n");
	led_set_brightness(&blue_led, 0);

	printf("\n--- Test brightness out of range (200) ---\n");
	led_set_brightness(&blue_led, 200);

	printf("\n--- Cleanup ---\n");
	led_deinit(&red_led);
	led_deinit(&green_led);
	led_deinit(&blue_led);

	printf("\n========================================\n");
	printf("  3 LEDs share 1 set of led_on/off/toggle code.\n");
	printf("  This is encapsulation in its simplest form.\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
