/* SPDX-License-Identifier: MIT */
/*
 * main.c - 启动入口
 *
 * 注意 main 里没有显式调用 led_init / uart_init / i2c_init / spi_init。
 * 加一个驱动文件 drv_xxx.c 写一行 MODULE_INIT(xxx_init)，main 一字不改。
 *
 * 这就是 Linux 内核的开闭原则：对扩展开放，对修改关闭。
 */

#include "initcall.h"
#include <stdio.h>

int main(void)
{
	printf("=========================================\n");
	printf("  ch17 - linker-time auto registration\n");
	printf("=========================================\n");

	printf("\n>>> kernel boot, run initcalls <<<\n\n");

	do_initcalls();

	printf("\n>>> all drivers ready <<<\n");
	printf("\n=========================================\n");
	printf("  main never references any drv_* function\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
