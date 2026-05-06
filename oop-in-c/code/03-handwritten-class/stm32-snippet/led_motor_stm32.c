/* SPDX-License-Identifier: MIT */
/*
 * led_motor_stm32.c - led + motor 的 platform 在 STM32 上长什么样
 *
 * 这是片段，不是完整工程。完整 STM32 工程见附录 B。
 *
 * 关键观察：led.h / led.c / motor.h / motor.c / main.c 全部一字不改。
 * 两个模块照样跑，前缀照样区分得开。变化的只是 platform 这层胶水。
 *
 * 真实工程里 motor 的 PWM 速度控制会写到 TIM 通道的 CCRx 寄存器，
 * 这里教学简化为 GPIO 高低电平。第 5 章会展开 HAL 怎么做映射。
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

/*
 * 应用层调用方式（和 PC 版完全一样）：
 *
 *   struct led red;
 *   struct motor fan;
 *   led_init(&red, 13);
 *   motor_init(&fan, 5);
 *   led_on(&red);
 *   motor_start(&fan);
 *
 * 命名前缀的好处在 STM32 上同样体现：
 * 三家芯片厂的 HAL 库共存（HAL_GPIO_xxx, BSP_LED_xxx, drv_motor_xxx），
 * 大家不会撞名。
 */
