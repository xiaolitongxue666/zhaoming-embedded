/* SPDX-License-Identifier: MIT */
/*
 * led_bad.c - 反面教材：5 个全局变量散在文件开头
 *
 * 致命问题：
 *   两个 LED（pin=5 和 pin=3）共享同一个 g_pin。
 *   bad_led_init(3) 后，g_pin 从 5 变成 3 -> 红灯的 pin 被覆盖。
 *   之后操作"红灯"，实际操作的是 pin=3 (绿灯的引脚)。
 *
 * 这种 bug 隐蔽在于代码不会报错，逻辑全错。
 * 你查一周也查不出来。
 *
 * 数据没有主人，bug 就是主人。
 */

#include "led_bad.h"
#include <stdio.h>

/*
 * 5 个全局变量散在文件开头：
 *
 *   g_pin / g_brightness  -- 实例数据用全局变量（最致命）
 *   init_count            -- 模块数据没加 static（外部 extern 能改）
 *   MAX_BRIGHTNESS        -- 用变量存常量（运行时能改）
 *   g_debug_flag          -- 调试开关没加 static
 */

int g_pin = 0;
int g_brightness = 0;
int init_count = 0;
int MAX_BRIGHTNESS = 255;
int g_debug_flag = 0;

int bad_led_init(uint8_t pin)
{
	g_pin = pin;
	g_brightness = 0;
	init_count++;

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, false);

	printf("  [BAD_LED] Pin%u initialized (g_pin=%d, init #%d)\n",
	       (unsigned)pin, g_pin, init_count);
	return 0;
}

int bad_led_on(void)
{
	/* 用的是 g_pin -- 如果被覆盖了，操作的就是别人的引脚 */
	platform_gpio_write((uint8_t)g_pin, true);
	printf("  [BAD_LED] Pin%d ON\n", g_pin);
	return 0;
}

int bad_led_off(void)
{
	platform_gpio_write((uint8_t)g_pin, false);
	printf("  [BAD_LED] Pin%d OFF\n", g_pin);
	return 0;
}

int bad_led_set_brightness(uint8_t brightness)
{
	if (brightness > MAX_BRIGHTNESS) {
		printf("  [BAD_LED] Error: brightness %u out of range (0~%d)\n",
		       (unsigned)brightness, MAX_BRIGHTNESS);
		return -1;
	}
	g_brightness = brightness;
	printf("  [BAD_LED] brightness set to %d (MAX=%d)\n",
	       g_brightness, MAX_BRIGHTNESS);
	return 0;
}

int bad_led_get_pin(void)
{
	return g_pin;
}

int bad_led_get_brightness(void)
{
	return g_brightness;
}
