/* SPDX-License-Identifier: MIT */
/*
 * main.c - 应用层
 *
 * ch13 主题 container_of 是子类内部的事 (子类 .c 用 container_of 反推
 * 子类指针). 应用层一字不知道, 只 include leds.h, 拿三个 struct led_base *
 * 句柄. 这一层封装跟 ch12 完全一致, 不回退.
 *
 * grep 验证 (在本目录跑):
 *   grep -n "led_gpio\|led_pwm\|led_i2c" main.c    # 0 行
 *   grep -n "gpio_write\|pwm_\|i2c_"      main.c   # 0 行
 *   grep -n "container_of"                main.c   # 0 行 (是子类内部的事)
 */

#include "leds.h"
#include <stdio.h>

int main(void)
{
	int rc;

	printf("=========================================\n");
	printf("  ch13 - container_of\n");
	printf("=========================================\n");

	rc = led_board_init();
	if (rc != 0) {
		printf("led_board_init failed, rc=%d\n", rc);
		return rc;
	}

	struct led_base *handles[] = {
		g_led_error,
		g_led_status,
		g_led_network,
	};

	for (int i = 0; i < 3; i++) {
		printf("\n--- toggle %s ---\n", handles[i]->name);
		led_on(handles[i]);
		led_off(handles[i]);
	}

	printf("\n--- breath stat ---\n");
	led_set_brightness(g_led_status, 60);
	led_set_brightness(g_led_status, 0);

	printf("\n=========================================\n");
	printf("  base offset != 0 still works\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
