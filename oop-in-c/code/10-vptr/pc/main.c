/* SPDX-License-Identifier: MIT */
/*
 * main.c - 每颗 LED 自带 ops 表
 *
 * 调用方拿到 base 指针就够了, 不用再传 ops 表 --
 * 那张表跟着 me 自己跑. test_led 内部直接 me->ops->on(me).
 */

#include <stdio.h>
#include "led.h"

int main(void)
{
	struct led_gpio red_led;
	struct led_pwm  blue_led;
	struct led_base *me;

	printf("========================================\n");
	printf("  ops field lands inside led_base.\n");
	printf("  Each LED carries its own ops table.\n");
	printf("========================================\n\n");

	led_gpio_init(&red_led, "red", 13);
	led_pwm_init(&blue_led, "blue", 1, 70);

	printf("\n--- test_led(&red_led.base) ---\n");
	test_led(&red_led.base);   /* 内部 me->ops->on(me) */

	printf("\n--- test_led(&blue_led.base) ---\n");
	test_led(&blue_led.base);  /* 同一份 test_led, ops 各自 */

	/* 直接调用形态: 拿到 base 指针后走 me->ops->on(me) */
	printf("\n--- Direct: me->ops->on(me) ---\n");
	me = &red_led.base;
	me->ops->on(me);
	me->ops->off(me);

	printf("\n========================================\n");
	printf("  Each object carries its ops pointer.\n");
	printf("  Call site: me->ops->on(me).\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
