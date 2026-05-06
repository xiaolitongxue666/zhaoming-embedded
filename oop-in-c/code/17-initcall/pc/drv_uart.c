/* SPDX-License-Identifier: MIT */
/*
 * drv_uart.c - UART 驱动模块（自注册）
 *
 * 整个文件 main 看不见、也 grep 不到 uart_init 这个名字。
 * 通过 MODULE_INIT 把指针塞进 my_initcall 段，启动期被自动调到。
 * 加新驱动只需要新建一个这样的文件，main.c 0 改动。见 ch17。
 */
#include "initcall.h"
#include <stdio.h>

static int uart_init(void)
{
	printf("    [uart]   uart_init: register UART driver\n");
	return 0;
}

MODULE_INIT(uart_init);
