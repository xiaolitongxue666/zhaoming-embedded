/* SPDX-License-Identifier: MIT */
/*
 * drv_led.c - LED 驱动模块
 *
 * 这个文件里没有任何对 main 或别人的函数引用，它通过 MODULE_INIT
 * 注册自己。启动期 do_initcalls 会扫到 led_init 并调到。
 *
 * Linux 内核的 module_init(fn) 宏，最终也展开到这个机制。
 * 见 ch17 § 17.2 module_init 是个魔法吗。
 */

#include "initcall.h"
#include <stdio.h>

static int led_init(void)
{
	printf("    [led]    led_init: register LED driver\n");
	return 0;
}

MODULE_INIT(led_init);
