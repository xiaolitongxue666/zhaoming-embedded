/* SPDX-License-Identifier: MIT */
/*
 * drv_i2c.c - I2C 驱动模块（自注册，同 drv_uart.c 套路）。见 ch17。
 */
#include "initcall.h"
#include <stdio.h>

static int i2c_init(void)
{
	printf("    [i2c]    i2c_init: register I2C driver\n");
	return 0;
}

MODULE_INIT(i2c_init);
