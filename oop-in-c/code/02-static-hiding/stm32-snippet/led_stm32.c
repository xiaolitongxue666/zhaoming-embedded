/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - 同一份 led_create / led_on 在 STM32 上长什么样
 *
 * 这是片段，不是完整工程。完整 STM32 工程见附录 B。
 *
 * 关键观察：led.h / led.c / main.c 一字不改。
 * 字段还藏在 led.c 里，外部 main.c 看不到 pin / brightness / is_on。
 * 变化的只是下面这层 platform_*.c —— 平台抽象层最直接的威力。
 */

#include "platform.h"
#include "stm32f4xx_hal.h"

/*
 * 假设硬件上 Pin 13/14/15 接到了 GPIOA 的 PA13/PA14/PA15。
 * 真实工程里这个映射会做成查找表，这里写死最直观。
 */

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

/*
 * 应用层调用方式（和 PC 版完全一样）：
 *
 *   struct led *red = led_create(13);
 *   led_on(red);
 *   red->pin = 999;        // 编译报错: invalid use of undefined type
 *   led_destroy(red);
 *
 * 信息隐藏在 ARM Cortex-M 上和 x86 上行为完全一致：
 * forward declaration 是编译期机制，编译过不去就生成不了 .o，烧录都没机会。
 */
