/* SPDX-License-Identifier: MIT */
/*
 * ch14 STM32 等效片段：替换 ch14 pc/ 里 PC 模拟版的 platform 封装函数。
 * led.c / main.c / container_of.h 一字不动。
 *
 * 必填 / 选填 / 接口三种 ops 表策略不依赖平台——assert 在 STM32 调试构建
 * 里照样把"忘填"暴露给你；Release 构建 -DNDEBUG 后 assert 整行编译消失。
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
