/* SPDX-License-Identifier: MIT */
/*
 * main.c - "提公因式" 之后的样子
 *
 * struct led_gpio 和 struct led_pwm 把 name + is_on 这两个公共字段
 * 提到 struct led_base. 子类的 init 第一行调 led_base_init,
 * 一处定义, 多处复用.
 *
 * 也演示: 因为 base 放在第一个位置, 拿到子类指针, 直接 &me->base
 * 就能交给只接受 struct led_base * 的函数 (led_base_get_name).
 * 第 12 章再展开为什么这件事重要.
 */

#include <stdio.h>
#include "led_gpio.h"
#include "led_pwm.h"

int main(void)
{
	struct led_gpio red_led;
	struct led_pwm  blue_led;

	printf("========================================\n");
	printf("  Inherit pain: name + is_on in led_base.\n");
	printf("  pin / channel live in their sub-classes.\n");
	printf("========================================\n\n");

	printf("--- Init led_gpio \"red\" on PA.13 ---\n");
	led_gpio_init(&red_led, "red", PIN_NUM('A', 13));   /* 0x0D = PA.13 */

	printf("\n--- Init led_pwm \"blue\" on channel 1 ---\n");
	led_pwm_init(&blue_led, "blue", 1, 70);

	printf("\n--- Drive red (GPIO) ---\n");
	led_gpio_on(&red_led);
	led_gpio_off(&red_led);

	printf("\n--- Drive blue (PWM) ---\n");
	led_pwm_on(&blue_led);
	led_pwm_off(&blue_led);

	printf("\n--- Common base API works for both ---\n");
	printf("  red name  = %s, is_on=%s\n",
	       led_base_get_name(&red_led.base),
	       led_base_is_on(&red_led.base) ? "true" : "false");
	printf("  blue name = %s, is_on=%s\n",
	       led_base_get_name(&blue_led.base),
	       led_base_is_on(&blue_led.base) ? "true" : "false");

	printf("\n========================================\n");
	printf("  Common fields defined once, used twice.\n");
	printf("  Sub-class init chains base init first.\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
