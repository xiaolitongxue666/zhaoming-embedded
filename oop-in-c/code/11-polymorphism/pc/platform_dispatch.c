/* SPDX-License-Identifier: MIT */
/**
 * @file  platform_dispatch.c
 * @brief platform 层封装函数 + 内部 ops 分发 - "对外稳定, 对内可换"
 *
 * @details
 * 这个文件做两件事:
 *
 *   1. 实现 common/platform.h 里声明的封装函数:
 *        platform_gpio_init / platform_gpio_write /
 *        platform_gpio_read / platform_gpio_deinit
 *      签名跟 ch01 起一字不变. 函数体走 g_platform_ops->xxx(...)
 *      dispatch, 自动落到当前选定的平台实现.
 *
 *   2. 实现 platform_ops.h 里声明的切换 + 查询 API:
 *        platform_select / platform_name
 *
 * 驱动层、应用层永远只调封装函数, 看不到 g_platform_ops 这个内部
 * 指针. 这是工业代码"对外是普通 C 函数, 对内可以是任何实现"的
 * 标准做法.
 *
 * 这一层是 ch11 § 11.5 演化的核心. 设备层 led 一字不知, 但底下的
 * platform_gpio_write 已经从"直接 BL 跳到一个固定函数"变成"两次
 * LDR + BLX 跳到当前 ops 选定的实现".
 */

#include "platform.h"
#include "platform_ops.h"
#include <stdio.h>

/*
 * 内部 ops 指针. 文件作用域 + static, 外部不可见. 默认指向 PC 实例,
 * main 启动后可通过 platform_select 切换 (比如切到 STM32 / Linux /
 * mock 实例).
 *
 * static 这一层隐藏的不是数据, 而是 API 表面: 整个程序只能通过
 * platform_select 改这个指针, 不允许其他模块直接 g_platform_ops = ...,
 * 防止平台切换被误用. 见 ch02 信息隐藏 / ch10 § 10.11 工业纪律.
 */
static const struct platform_ops *g_platform_ops = &platform_pc;

void platform_select(const struct platform_ops *p)
{
	if (p) {
		g_platform_ops = p;
		printf("[platform] selected: %s\n", p->name);
	}
}

const char *platform_name(void)
{
	return g_platform_ops ? g_platform_ops->name : "(unset)";
}

/*
 * ============== 封装函数: 内部走 ops dispatch ==============
 *
 * 签名跟 ch01 起一字不变. 驱动层调用形态不变, 多出来的两次 LDR
 * 在这一层内部消化. 函数体两个 NULL check (ops 指针、字段) 都不
 * 能少: 没初始化就调直接跳到地址 0 = HardFault/SIGSEGV.
 *
 * 把 dispatch 集中在这里、不让驱动层直接走 g_platform_ops 的两个
 * 理由 (跟 ch11 § 11.6.1 同源):
 *   1) 集中 NULL check
 *   2) API 稳定 - 加日志、trace、统计都改这一层, 驱动层不动
 */

void platform_gpio_init(uint8_t pin, uint8_t mode)
{
	if (g_platform_ops && g_platform_ops->gpio_init)
		g_platform_ops->gpio_init(pin, mode);
}

void platform_gpio_deinit(uint8_t pin)
{
	if (g_platform_ops && g_platform_ops->gpio_deinit)
		g_platform_ops->gpio_deinit(pin);
}

void platform_gpio_write(uint8_t pin, bool value)
{
	if (g_platform_ops && g_platform_ops->gpio_write)
		g_platform_ops->gpio_write(pin, value);
}

bool platform_gpio_read(uint8_t pin)
{
	if (g_platform_ops && g_platform_ops->gpio_read)
		return g_platform_ops->gpio_read(pin);
	return false;
}
