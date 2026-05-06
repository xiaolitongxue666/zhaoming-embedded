/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_stm32_mock.c - STM32 平台的"假装版"
 *
 * 真正 STM32 上这一份会调 HAL_GPIO_WritePin。在 PC 上为了演示
 * "运行时切换平台"，它只是把动作打到屏幕，标记 [STM32]，伪装成
 * STM32 在跑。这样 PC 上一个 demo 进程能把三家 platform（PC / STM32 /
 * Linux）轮流切一遍，验证"换平台 = 换 ops 实例，上层不改"。
 *
 * 真实 STM32 工程的对应版本见 stm32-snippet/platform_ops_stm32.c。
 * 两份的接口签名（platform_ops 字段集合）完全一样，应用层 / led 层 /
 * board 层一字不改。见 ch15 § 15.7 mock 三连切换。
 */

#include "platform_ops.h"
#include <stdio.h>

static void stm32_gpio_init(uint8_t pin, uint8_t mode)
{
	const char *s = (mode == 0) ? "OUTPUT" : "INPUT";
	printf("    [STM32] Pin%u init as %s (config GPIOA MODER)\n",
	       (unsigned)pin, s);
}

static void stm32_gpio_write(uint8_t pin, bool value)
{
	/*
	 * 真机上这里就是：
	 *   GPIOA->BSRR = value ? (1u << pin) : (1u << (pin + 16));
	 * BSRR 一次 32 位 store，原子。
	 */
	printf("    [STM32] BSRR <- 0x%08X (Pin%u %s)\n",
	       value ? (1u << pin) : (1u << (pin + 16)),
	       (unsigned)pin, value ? "HIGH" : "LOW");
}

static bool stm32_gpio_read(uint8_t pin)
{
	(void)pin;
	return false;
}

static void stm32_gpio_deinit(uint8_t pin)
{
	printf("    [STM32] Pin%u config back to analog\n", (unsigned)pin);
}

const struct platform_ops platform_stm32_mock = {
	.name        = "STM32",
	.gpio_init   = stm32_gpio_init,
	.gpio_write  = stm32_gpio_write,
	.gpio_read   = stm32_gpio_read,
	.gpio_deinit = stm32_gpio_deinit,
};
