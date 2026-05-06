/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_stm32.c - STM32 HAL 等效片段
 *
 * 不是完整工程。把 ch12 PC 版的 platform_ops_pc.c 替换成这一份，
 * 上层 led.c / board_init.c / main.c 一字不改。
 *
 * 完整 STM32 工程见附录 B。
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
