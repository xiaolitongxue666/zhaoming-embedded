/* SPDX-License-Identifier: MIT */
/*
 * main.c - 山寨内核启动 + 同一份 leds-gpio 跑两家芯片
 */

#include "gpio_chip.h"
#include <stdio.h>
#include <stdlib.h>

void vendor_a_probe(void);
void vendor_b_probe(void);
void led_gpio_brightness_set(struct gpio_desc *desc, int value);

int main(void)
{
	printf("=========================================\n");
	printf("  ch16 - linux-style gpio subsystem\n");
	printf("=========================================\n");

	/* 启动期注册 chip。真实内核里走 module_init。 */
	vendor_a_probe();
	vendor_b_probe();

	/* leds-gpio 驱动通过 chip + offset 拿到 desc */
	struct gpio_desc *led_red   = gpio_get_desc("vendorA-gpio", 5);
	struct gpio_desc *led_green = gpio_get_desc("vendorB-gpio", 2);

	printf("\n--- leds-gpio drives both chips ---\n");
	led_gpio_brightness_set(led_red,   1);
	led_gpio_brightness_set(led_green, 1);
	led_gpio_brightness_set(led_red,   0);
	led_gpio_brightness_set(led_green, 0);

	printf("\n>>> same gpiod_set_value() dispatches to two vendors <<<\n");

	free(led_red);
	free(led_green);

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
