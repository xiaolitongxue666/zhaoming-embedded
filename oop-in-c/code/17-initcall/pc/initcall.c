/* SPDX-License-Identifier: MIT */
/*
 * initcall.c - 启动期遍历 my_initcall 段
 *
 * 这一份是 ch17 整套机制的"运行时收尾"。前面 MODULE_INIT 已经把每个
 * 驱动的 _init 函数指针塞进 my_initcall 段，链接器把所有 .o 的同名段
 * 合并成一个连续区域。剩下的就是启动期遍历这个区间，挨个调过去。
 *
 * main.c 不需要 #include "drv_uart.h"、不需要 grep 出所有驱动名、
 * 不需要写一长串 xxx_init() 调用。加新驱动 = 写新文件 + 一行
 * MODULE_INIT(xxx_init)，链接器自动把它加进段，启动期 do_initcalls
 * 自动调到。这就是 Linux 内核 module_init 那一行魔法的全部秘密。
 *
 * 真实内核的 do_initcalls 在 init/main.c 第 1297 行：
 *
 *   static void __init do_initcalls(void)
 *   {
 *       int level;
 *       ...
 *       for (level = 0; level < ARRAY_SIZE(initcall_levels) - 1; level++) {
 *           ...
 *           do_initcall_level(level, command_line);
 *       }
 *   }
 *
 * 我们的版本砍掉级别和命令行，直接遍历从 __start 到 __stop。
 *
 * 见 ch17 § 17.4 do_initcalls + § 17.5 演示。
 */

#include "initcall.h"
#include <stdio.h>

void do_initcalls(void)
{
	initcall_t *fn;

	printf("[do_initcalls] sweep .my_initcall section "
	       "from %p to %p\n",
	       (void *)__start_my_initcall, (void *)__stop_my_initcall);

	/* 段就是一个连续 initcall_t 数组：从 __start_my_initcall 走到
	 * __stop_my_initcall，每一项就是某个驱动当年用 MODULE_INIT 注册的
	 * _init 函数指针。本 main.c 完全不知道哪些驱动被链进来，链接器
	 * 帮它把名字都解析掉了。
	 */
	for (fn = __start_my_initcall; fn < __stop_my_initcall; fn++) {
		printf("[do_initcalls] call %p\n", (void *)*fn);
		(*fn)();
	}

	printf("[do_initcalls] done, %ld initcalls\n",
	       (long)(__stop_my_initcall - __start_my_initcall));
}
