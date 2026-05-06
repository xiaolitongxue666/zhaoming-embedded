/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - 共性提取后, STM32 端长什么样
 *
 * 这是片段, 不是完整工程. 完整工程见附录 B.
 *
 * 关键观察: 把 pin 放到 led_base, 子类的 STM32 端实现一行不变,
 * led.c / motor.c / main.c 也一字不改. 平台胶水 platform_*
 * 还是 ch01-ch05 的函数式形态.
 */

#include "led.h"
#include "motor.h"
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

/*
 * 应用层 (节选):
 *
 *   struct led red_led;
 *   struct motor fan;
 *
 *   led_init(&red_led, 13);     // 第一行 led_base_init, 配 PA13
 *   motor_init(&fan, 22);       // 第一行 led_base_init, 配 PA6
 *   led_on(&red_led);
 *   motor_set_duty(&fan, 75);
 *
 * 跨芯片移植: 只重写 platform_gpio_*, 上层一字不改.
 */
