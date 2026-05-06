/* SPDX-License-Identifier: MIT */
/*
 * platform_dispatch.c - platform 层封装函数 + 内部 ops 分发
 *
 * 这个文件做两件事：
 *
 *   1. 实现 common/platform.h 里声明的封装函数：
 *        platform_gpio_init / platform_gpio_write /
 *        platform_gpio_read / platform_gpio_deinit
 *      封装函数体内部走 g_platform_ops->xxx(...) dispatch，自动落到
 *      当前选定的平台实现。
 *
 *   2. 实现 platform_ops.h 里声明的切换 + 查询 API：
 *        platform_select / platform_name
 *
 * 驱动层、应用层永远只调封装函数，看不到 g_platform_ops 这个内部指针。
 * 这是工业代码里的标准做法："对外是普通 C 函数，对内可以是任何实现"。
 *
 * 这一层是"换硬件不改应用"在 C 里的真实落地——main.c 里调一次
 * platform_select(&platform_stm32_mock)，整个进程的 GPIO 动作就全部
 * 重定向到 STM32 实现，应用代码 0 行改动。见 ch15 § 15.5 + § 15.7。
 */

#include "platform.h"
#include "platform_ops.h"
#include <stdio.h>

/*
 * 内部 ops 指针。文件作用域 static，外部不可见——驱动层 grep 不到、
 * 也碰不到这个指针。默认指向 PC 实例，main.c 启动后通过 platform_select
 * 切换。这就是"对内可换"的核心：换平台只是改这一个指针，封装函数体
 * 不变，所有上层调用点不变。
 */
static const struct platform_ops *g_platform_ops = &platform_pc;

void platform_select(const struct platform_ops *p)
{
	if (p) {
		g_platform_ops = p;
		printf(">>> platform switched to: %s\n", p->name);
	}
}

const char *platform_name(void)
{
	return g_platform_ops ? g_platform_ops->name : "(unset)";
}

/* ============== 封装函数：内部走 ops dispatch ============== */

void platform_gpio_init(uint8_t pin, uint8_t mode)
{
	if (g_platform_ops && g_platform_ops->gpio_init)
		g_platform_ops->gpio_init(pin, mode);
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

void platform_gpio_deinit(uint8_t pin)
{
	if (g_platform_ops && g_platform_ops->gpio_deinit)
		g_platform_ops->gpio_deinit(pin);
}
