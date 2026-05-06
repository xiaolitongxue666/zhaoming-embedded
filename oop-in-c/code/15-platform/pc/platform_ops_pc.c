/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_pc.c - PC 模拟版 platform_ops 实例
 *
 * 这个文件只定义一个 const 实例，不维护任何全局状态——所有静态函数
 * 直接 printf。切换实例的逻辑在 platform_dispatch.c 里，本文件不参与
 * 切换决策。这是 ch15 的"对外封装、对内 ops"分层：每个平台实例只
 * 关心自己怎么实现，分发逻辑统一交给 dispatch。
 *
 * const 修饰整个 struct 让链接器把它放到 .rodata 段，运行时只读，
 * 防误改。见 ch15 § 15.5 platform 层 ops 化。
 */

#include "platform_ops.h"
#include <stdio.h>

static void pc_gpio_init(uint8_t pin, uint8_t mode)
{
	const char *s = (mode == 0) ? "OUTPUT" : "INPUT";
	printf("    [PC]    Pin%u init as %s\n", (unsigned)pin, s);
}

static void pc_gpio_write(uint8_t pin, bool value)
{
	printf("    [PC]    Pin%u <- %s\n", (unsigned)pin,
	       value ? "HIGH" : "LOW");
}

static bool pc_gpio_read(uint8_t pin)
{
	(void)pin;
	return false;
}

static void pc_gpio_deinit(uint8_t pin)
{
	printf("    [PC]    Pin%u released\n", (unsigned)pin);
}

const struct platform_ops platform_pc = {
	.name        = "PC",
	.gpio_init   = pc_gpio_init,
	.gpio_write  = pc_gpio_write,
	.gpio_read   = pc_gpio_read,
	.gpio_deinit = pc_gpio_deinit,
};
