/* SPDX-License-Identifier: MIT */
/*
 * main.c - 父类统一接口 led_on / led_off / led_toggle
 *
 * 三种 LED (GPIO / PWM / I2C) 装进 struct led_base * 数组. 同一行
 * led_on(base) 跑出三种行为: 红灯走 gpio_on, 蓝灯走 pwm_on, 绿灯走
 * i2c_on. 应用层一字不知谁是谁, ops 字段挂在每颗 LED 自己身上.
 *
 * 这是 ch11 第一次同时演示三种子类 -- ch06 提共性时已经预告"GPIO /
 * PWM / I2C 三种子类共用 base", 这一章的多态 dispatch 把这件事兑现.
 */

#include <stdio.h>
#include "led_base.h"
#include "led_gpio.h"
#include "led_pwm.h"
#include "led_i2c.h"
#include "platform.h"

int main(void)
{
	struct led_gpio red_led;
	struct led_pwm  blue_led;
	struct led_i2c  green_led;

	printf("========================================\n");
	printf("  led_on / led_off / led_toggle\n");
	printf("  Same call, three behaviors per LED kind.\n");
	printf("========================================\n\n");

	led_gpio_init(&red_led,   "red",   PIN_NUM('A', 13));
	led_pwm_init (&blue_led,  "blue",   1, 70);
	led_i2c_init (&green_led, "green", 0x20, 0x01);

	/* base 指针数组: 三种 LED 装进同一个数组 */
	struct led_base *all_leds[] = {
		&red_led.base,
		&blue_led.base,
		&green_led.base,
	};
	int n = sizeof(all_leds) / sizeof(all_leds[0]);

	printf("\n--- Loop over led_base * array, call led_on ---\n");
	for (int i = 0; i < n; ++i) {
		printf(" idx=%d:\n", i);
		led_on(all_leds[i]);
	}

	printf("\n--- Loop, call led_off ---\n");
	for (int i = 0; i < n; ++i) {
		printf(" idx=%d:\n", i);
		led_off(all_leds[i]);
	}

	printf("\n========================================\n");
	printf("  One led_on(base) line, three behaviors:\n");
	printf("    [0] red    -> gpio_on   (pull GPIO pin)\n");
	printf("    [1] blue   -> pwm_on    (set duty)\n");
	printf("    [2] green  -> i2c_on    (I2C write reg)\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
