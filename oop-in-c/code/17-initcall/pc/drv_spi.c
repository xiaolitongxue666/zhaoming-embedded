/* SPDX-License-Identifier: MIT */
/*
 * drv_spi.c - SPI 驱动模块（自注册，同 drv_uart.c 套路）
 * main.c 看不见 spi_init 名字，链接器把它收集进 my_initcall 段。见 ch17。
 */
#include "initcall.h"
#include <stdio.h>

static int spi_init(void)
{
	printf("    [spi]    spi_init: register SPI driver\n");
	return 0;
}

MODULE_INIT(spi_init);
