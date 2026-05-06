/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - 函数指针字段在 STM32 上的样子
 *
 * led.h / led.c / main.c 一字不改. 平台胶水继续 ch01 的函数式包装.
 *
 * 一个真实场景: 同一颗 LED 引脚, 调试模式下用 gpio-style (亮即开),
 * 量产模式下用 pwm-style (按 brightness 调光). 出厂烧录决定填哪个
 * on_func, 不重新编译 led.c.
 */

#include "led.h"
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
 * 调用 (节选):
 *
 *   struct led red_led;
 *   led_init(&red_led, 13, led_on_gpio_style);   // 简单模式
 *   led_on(&red_led);
 *
 *   // 调试模式想换成 pwm-style 不需要重新编译 led.c
 *   red_led.on_func = led_on_pwm_style;
 */
