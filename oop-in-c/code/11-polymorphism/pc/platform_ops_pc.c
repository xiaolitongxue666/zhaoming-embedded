/* SPDX-License-Identifier: MIT */
/**
 * @file  platform_ops_pc.c
 * @brief PC 模拟版 platform_ops 实例
 *
 * @details
 * 这个文件只定义一个 const 实例 platform_pc + 内部 static 函数,
 * 不维护任何全局状态. 切换实例的逻辑 (g_platform_ops 指针 +
 * platform_select + 封装函数) 在 platform_dispatch.c 里.
 *
 * 实现函数全部 static -- 它们的唯一对外入口是 platform_pc 这张
 * ops 表. 应用层、驱动层、测试用例都不该直接调 pc_gpio_init,
 * 也根本调不到 (static 链接期就藏起来). 见 ch02 § 2.x static 信息隐藏.
 *
 * 这是 ch11 起的"对外封装、对内 ops"分层. 整个 platform 层有两个
 * 文件:
 *   - platform_ops_pc.c (本文件): 实例定义
 *   - platform_dispatch.c        : 切换 + 封装函数 dispatch
 */

#include "platform_ops.h"
#include <stdio.h>

static void pc_gpio_init(uint8_t pin, uint8_t mode)
{
	const char *m = (mode == 0) ? "OUTPUT" : "INPUT";
	printf("[GPIO] Pin%u init as %s\n", (unsigned)pin, m);
}

static void pc_gpio_deinit(uint8_t pin)
{
	printf("[GPIO] Pin%u released\n", (unsigned)pin);
}

static void pc_gpio_write(uint8_t pin, bool value)
{
	printf("[GPIO] Pin%u -> %s\n", (unsigned)pin,
	       value ? "HIGH (ON)" : "LOW (OFF)");
}

static bool pc_gpio_read(uint8_t pin)
{
	printf("[GPIO] Read Pin%u -> LOW\n", (unsigned)pin);
	return false;
}

/*
 * platform_pc - PC 模拟版的 ops 表.
 *
 * const 修饰让它落 .rodata, 链接期不可改, 防止 g_platform_ops 错
 * 指向之后被无意改写. designated initializer 让没列出的字段自动
 * 填 NULL (本表全部填齐). 见 ch09 § 9.5.1.
 */
const struct platform_ops platform_pc = {
	.name        = "PC",
	.gpio_init   = pc_gpio_init,
	.gpio_deinit = pc_gpio_deinit,
	.gpio_write  = pc_gpio_write,
	.gpio_read   = pc_gpio_read,
};
