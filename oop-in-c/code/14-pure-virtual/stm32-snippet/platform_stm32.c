/* SPDX-License-Identifier: MIT */
/*
 * platform_stm32.c - ch14 STM32 等效片段（函数式 platform）
 *
 * 替换 ch14 pc/ 里 PC 模拟版的 platform 封装函数实现。led.c / main.c /
 * container_of.h 一字不动。
 *
 * 必填 / 选填 / 全必填三种 ops 表策略不依赖平台。assert 在 STM32 调试
 * 构建里照样把"忘填"暴露给你；Release 构建定义 NDEBUG 后 assert 整行
 * 编译消失。本章主线讨论的是 led_ops 这一层（子类层）的策略，platform
 * 只是稳定背景，所以这里直接用 4 个函数把 HAL 包一层即可。
 *
 * platform 层从函数式演化成 ops 表是 ch15 的主题。
 */

#include "platform.h"
#include "stm32f4xx_hal.h"

void platform_gpio_init(uint8_t pin, uint8_t mode)
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

void platform_gpio_deinit(uint8_t pin)
{
	HAL_GPIO_DeInit(GPIOA, (uint16_t)(1U << pin));
}

void platform_gpio_write(uint8_t pin, bool value)
{
	HAL_GPIO_WritePin(GPIOA, (uint16_t)(1U << pin),
			  value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool platform_gpio_read(uint8_t pin)
{
	return HAL_GPIO_ReadPin(GPIOA, (uint16_t)(1U << pin)) == GPIO_PIN_SET;
}
