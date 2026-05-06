/* SPDX-License-Identifier: MIT */
/*
 * main.c - 反面教材 vs 正面教材
 *
 * Part 1 跑 led_bad（全局变量满天飞），第二次 init 覆盖第一次的
 *        g_pin，操作"红灯"实际操作的是绿灯的引脚。bug 演示。
 *
 * Part 2 跑 led（数据归位完成形态）。同样两个 LED，每个有自己的
 *        pin 字段，互不干扰。还演示静态对象池的耗尽行为。
 */

#include <stdio.h>
#include "led_bad.h"
#include "led.h"

int main(void)
{
	printf("========================================\n");
	printf("  Part 1: BAD code with global g_pin\n");
	printf("========================================\n\n");

	printf("--- bad_led_init for red (pin=5) ---\n");
	bad_led_init(5);
	printf("  g_pin = %d (correct)\n\n", bad_led_get_pin());

	printf("--- bad_led_init for green (pin=3) ---\n");
	bad_led_init(3);
	printf("  g_pin = %d (overwritten by second init)\n\n",
	       bad_led_get_pin());

	printf("--- Try to turn ON \"red\" ---\n");
	bad_led_on();
	printf("  But the actual pin is %d (green's pin), not 5.\n",
	       bad_led_get_pin());
	printf("  [BUG] g_pin is shared, two LEDs can't coexist.\n");

	printf("\n");
	printf("========================================\n");
	printf("  Part 2: GOOD code with struct + static pool\n");
	printf("========================================\n\n");

	printf("--- led_acquire for red (pin=5) ---\n");
	struct led *red = led_acquire(5);

	printf("\n--- led_acquire for green (pin=3) ---\n");
	struct led *green = led_acquire(3);

	printf("\n--- Both LEDs are independent ---\n");
	led_on(red);
	led_set_brightness(red, 80);
	led_on(green);
	led_set_brightness(green, 40);

	bool red_on, green_on;
	uint8_t red_b, green_b;
	led_get_state(red, &red_on, &red_b);
	led_get_state(green, &green_on, &green_b);
	printf("  red:   is_on=%s brightness=%u%%\n",
	       red_on ? "true" : "false", (unsigned)red_b);
	printf("  green: is_on=%s brightness=%u%%\n",
	       green_on ? "true" : "false", (unsigned)green_b);

	printf("\n--- Module-level data via function ---\n");
	printf("  led_get_init_count() = %u (no extern, no global var)\n",
	       led_get_init_count());

	printf("\n--- Pool exhaustion (acquire 8 then 9th fails) ---\n");
	struct led *spare[LED_POOL_SIZE];
	size_t n = 0;
	for (size_t i = 0; i < LED_POOL_SIZE - 2 + 1; i++) {
		spare[n] = led_acquire((uint8_t)(10 + i));
		if (!spare[n])
			break;
		n++;
	}
	struct led *overflow = led_acquire(99);
	printf("  9th acquire returned %p (NULL = pool exhausted, expected)\n",
	       (void *)overflow);

	printf("\n--- Cleanup ---\n");
	led_release(red);
	led_release(green);
	for (size_t i = 0; i < n; i++)
		led_release(spare[i]);

	printf("\n========================================\n");
	printf("  Data ownership in 3 steps:\n");
	printf("    pin / brightness  -> struct field (instance)\n");
	printf("    init count        -> static (module-private)\n");
	printf("    MAX_BRIGHTNESS    -> static const (read-only)\n");
	printf("    pool              -> static struct led led_pool[N]\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
