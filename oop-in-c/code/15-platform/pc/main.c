/* SPDX-License-Identifier: MIT */
/*
 * main.c - 运行时切换 platform 演示
 *
 * 同一份业务代码，跑三次，分别在 PC / STM32 / Linux 三种 platform 下执行。
 * 应用层、led 驱动层、board 层一字不改，只换 platform 全局指针。
 *
 * 这是 ch15 的核心：分层抽象的高潮。
 */

#include "app.h"
#include "leds.h"
#include "platform_ops.h"
#include <stdio.h>

int main(void)
{
	printf("=========================================\n");
	printf("  ch15 - platform abstraction climax\n");
	printf("=========================================\n");

	board_init();    /* 一次构造，三种 platform 通用 */

	printf("\n========== run on PC ==========\n");
	platform_select(&platform_pc);
	power_on_test();
	alarm_blink();
	status_breathe();

	printf("\n========== run on STM32 ==========\n");
	platform_select(&platform_stm32_mock);
	power_on_test();
	alarm_blink();
	status_breathe();

	printf("\n========== run on LINUX ==========\n");
	platform_select(&platform_linux_mock);
	power_on_test();
	alarm_blink();
	status_breathe();

	printf("\n=========================================\n");
	printf("  same app/led/board, 3 platforms\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
