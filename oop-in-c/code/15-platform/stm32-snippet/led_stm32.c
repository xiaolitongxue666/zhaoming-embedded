/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - 父类 / 子类 / 板级 / 应用 四层落到 STM32 上的样子
 *
 * ch15 完整框架在真实 STM32 工程里只换一份文件: 把 PC 模拟版的 4 个
 * platform 封装函数 (printf 模拟) 换成走 STM32 HAL 的真实实现. 父类
 * led.c / 子类 ops / 板级 board_init.c / 应用 app.c 一字不动.
 *
 * gpio_on 子类实现里的 platform_gpio_write(self->pin, self->on_level),
 * 在 STM32 上调到底就是 HAL_GPIO_WritePin -> GPIOx->BSRR 一次 32 位
 * store (原子). 见 ch15 § 15.10 在 STM32 上长什么样.
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
