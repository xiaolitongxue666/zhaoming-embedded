/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_stm32.c - 真实 STM32 上的 platform_ops 实例
 *
 * pc/ 里 platform_ops_stm32_mock.c 是用 printf 假装的版本，方便在 PC
 * 上演示运行时切换。这一份是真实硬件版本：底下调 STM32 HAL 的
 * HAL_GPIO_WritePin，最终落到 GPIOx->BSRR 一次 32 位 store（原子）。
 *
 * 接口签名跟 pc/ 完全一样，led.c / app.c / board_init.c 一字不动。
 * 见 ch15 § 15.7 / 附录 B。
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

const struct platform_ops platform_stm32 = {
	.name       = "STM32-real",
	.gpio_init  = stm32_gpio_init,
	.gpio_write = stm32_gpio_write,
	.gpio_read  = stm32_gpio_read,
};

const struct platform_ops *platform = &platform_stm32;
