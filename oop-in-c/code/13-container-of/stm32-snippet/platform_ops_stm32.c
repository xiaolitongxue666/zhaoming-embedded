/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_stm32.c - ch13 STM32 等效片段
 * 替换 ch13 pc/ 里的 PC 模拟实现（common/platform_pc.c），上层一字不改。
 *
 * STM32 上 platform_gpio_write 直接走 HAL_GPIO_WritePin，落到 GPIOx->BSRR
 * 一次 32 位 store（原子）。container_of 这一招在 STM32 上编译产物就是
 * ARM Cortex-M 的 SUB Rd, Rn, #imm 一条指令，零运行时开销。
 * 见 ch13 § 13.9 在 STM32 上长什么样。
 *
 * 注：此处 platform_ops 这一层是 ch15 才正式引入的内部 ops 表。本片段
 * 用同样的命名风格示范"换平台 = 换文件"，main.c 是同一份。
 */

#include "platform_ops.h"
#include "stm32f4xx_hal.h"

static void stm32_gpio_init(uint8_t pin, uint8_t mode)
{
	GPIO_InitTypeDef cfg = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE();

	cfg.Pin   = (uint16_t)(1U << pin);
	cfg.Mode  = (mode == GPIO_MODE_OUTPUT) ?
	            GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
	cfg.Pull  = GPIO_NOPULL;
	cfg.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &cfg);
}

static void stm32_gpio_write(uint8_t pin, bool value)
{
	HAL_GPIO_WritePin(GPIOA, (uint16_t)(1U << pin),
			  value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static bool stm32_gpio_read(uint8_t pin)
{
	return HAL_GPIO_ReadPin(GPIOA, (uint16_t)(1U << pin)) == GPIO_PIN_SET;
}

static const struct platform_ops stm32_ops = {
	.gpio_init  = stm32_gpio_init,
	.gpio_write = stm32_gpio_write,
	.gpio_read  = stm32_gpio_read,
};

const struct platform_ops *platform = &stm32_ops;
