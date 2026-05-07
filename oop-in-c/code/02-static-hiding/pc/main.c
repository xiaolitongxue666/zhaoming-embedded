/* SPDX-License-Identifier: MIT */
/*
 * main.c - "锁后厨 + 标 private 字段" 的演示
 *
 * 这一章相对 ch01 增量很小:
 *   - struct led red; 还是栈上分配，字段还在 led.h 公开
 *   - 字段每个挂了 "private" 注释，纪律上不准外部直接写
 *   - 工具函数 update_hardware / brightness_valid 加 static，
 *     在 led.c 之外，连名字都看不到，链接器直接拦
 *   - 多了一个 led_get_state API，外部要读字段也走 API
 *
 * 看下面两段被注释的代码:
 *   - 软 private (字段直接写) 编译能过，靠纪律 + review 拦
 *   - 硬 private (static 工具函数被外部调用) 链接器直接报错
 *
 * 理解这两层强度差别，就理解了 C 圈子工程现实。
 */

#include <stdio.h>
#include "led.h"

int main(void)
{
	struct led red;
	struct led green;
	bool is_on;
	uint8_t brightness;

	printf("========================================\n");
	printf("  ch02: lock the kitchen, mark fields private\n");
	printf("========================================\n\n");

	printf("--- Init two LEDs (struct on stack) ---\n");
	led_init(&red,   PIN_NUM('A', 13));   /* 0x0D = PA.13 */
	led_init(&green, PIN_NUM('A', 14));   /* 0x0E = PA.14 */

	printf("\n--- Turn both on ---\n");
	led_on(&red);
	led_on(&green);

	printf("\n--- Read state through led_get_state ---\n");
	led_get_state(&red, &is_on, &brightness);
	printf("  red:   is_on=%s brightness=%u%%\n",
	       is_on ? "true" : "false", (unsigned)brightness);
	led_get_state(&green, &is_on, &brightness);
	printf("  green: is_on=%s brightness=%u%%\n",
	       is_on ? "true" : "false", (unsigned)brightness);

	printf("\n--- Set brightness ---\n");
	led_set_brightness(&red, 75);
	led_set_brightness(&green, 30);

	printf("\n--- Out-of-range brightness rejected by API ---\n");
	int ret = led_set_brightness(&red, 200);
	printf("  led_set_brightness(red, 200) returned %d (-2 = out of range)\n", ret);

	printf("\n--- Toggle red ---\n");
	led_toggle(&red);
	led_get_state(&red, &is_on, NULL);
	printf("  red is_on=%s after toggle\n", is_on ? "true" : "false");

	/*
	 * --- 软 private 演示 (字段直接写) ---
	 *
	 * 下面这一行编译能过: C 编译器看到 .h 里的字段定义就允许访问。
	 * 拦它的是: "private" 注释 + 命名纪律 + code review。
	 * 真实项目里 reviewer 看到这种"长得不像 led_xxx"的写法直接打回。
	 *
	 *   red.pin = 999;
	 *
	 * 取消注释能编过, 这正是 C 软 private 的边界。
	 */

	/*
	 * --- 硬 private 演示 (static 工具函数被外部调) ---
	 *
	 * 下面这一行链接器直接拒绝。update_hardware 在 led.c 里加了
	 * static，外部 .o 的全局符号表里根本没这个名字。
	 *
	 *   update_hardware(&red);
	 *
	 * 取消注释会得到:
	 *   warning: implicit declaration of function 'update_hardware'
	 *   undefined reference to `update_hardware'
	 *
	 * 这是机制层硬锁，0 漏网概率。
	 */
	printf("\n--- Two locks, two strengths ---\n");
	printf("  Soft (fields):     \"private\" comment + naming + review\n");
	printf("  Hard (static fns): linker refuses cross-file access\n");

	printf("\n--- Cleanup ---\n");
	led_deinit(&red);
	led_deinit(&green);

	printf("\n========================================\n");
	printf("  Soft + Hard private = encapsulation in C\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
