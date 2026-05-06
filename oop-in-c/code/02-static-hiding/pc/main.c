/* SPDX-License-Identifier: MIT */
/*
 * main.c - "把字段藏起来" 的演示
 *
 * struct led 的字段在 led.c 里定义，led.h 只 forward declare。
 * 所以这个文件根本不知道 led 里有 pin / brightness / is_on。
 *
 * 想直接 me->pin = 999 ? 编译就过不去:
 *   error: invalid use of undefined type 'struct led'
 *
 * 改字段只能走 led_set_brightness 这种正经 API。
 * 锁起来不是不信任，是不让任何人手滑。
 */

#include <stdio.h>
#include "led.h"

int main(void)
{
	struct led *red, *green;
	bool is_on;
	uint8_t brightness;

	printf("========================================\n");
	printf("  Hide the fields. Talk through the API.\n");
	printf("========================================\n\n");

	printf("--- Create two LEDs ---\n");
	red   = led_create(13);
	green = led_create(14);

	printf("\n--- Turn both on ---\n");
	led_on(red);
	led_on(green);

	printf("\n--- Read state through led_get_state ---\n");
	led_get_state(red, &is_on, &brightness);
	printf("  red:   is_on=%s brightness=%u%%\n",
	       is_on ? "true" : "false", (unsigned)brightness);
	led_get_state(green, &is_on, &brightness);
	printf("  green: is_on=%s brightness=%u%%\n",
	       is_on ? "true" : "false", (unsigned)brightness);

	printf("\n--- Set brightness ---\n");
	led_set_brightness(red, 75);
	led_set_brightness(green, 30);

	printf("\n--- Out-of-range brightness rejected by API ---\n");
	int ret = led_set_brightness(red, 200);
	printf("  led_set_brightness(red, 200) returned %d (-2 = out of range)\n", ret);

	printf("\n--- Toggle red ---\n");
	led_toggle(red);
	led_get_state(red, &is_on, NULL);
	printf("  red is_on=%s after toggle\n", is_on ? "true" : "false");

	/*
	 * 下面这一行如果取消注释，编译就会失败：
	 *   red->pin = 999;
	 *   error: invalid use of undefined type 'struct led'
	 *
	 * 为什么？因为 main.c 只 include 了 led.h，led.h 里只有
	 *   struct led;
	 * 这个前向声明，编译器根本不知道有没有 pin 字段。
	 * 字段被关进了 led.c，外部碰不到。
	 */
	printf("\n--- Direct field access blocked at compile time ---\n");
	printf("  Try uncommenting `red->pin = 999;` in main.c\n");
	printf("  gcc will refuse to compile (struct is incomplete here).\n");

	printf("\n--- Cleanup ---\n");
	led_destroy(red);
	led_destroy(green);

	printf("\n========================================\n");
	printf("  Fields locked in led.c. API is the only door.\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
