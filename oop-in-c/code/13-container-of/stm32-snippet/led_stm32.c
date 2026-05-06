/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - ch13 STM32 等效片段（函数式包装版）
 *
 * 父类 led_on / led_off / led_set_brightness 写在 led.c, 子类实现里
 * 第一行用 container_of 反推自己, 后面调 platform_gpio_xxx 封装函数.
 * STM32 上这一层落到 HAL_GPIO_xxx, 应用层 / 父类 / 子类一字不改.
 *
 * 跟 pc/ 唯一的差别就在这个文件: 把 printf 模拟换成真实 HAL 操作.
 *
 * container_of 这一招在 STM32 上编译产物就是 ARM Cortex-M 的
 * SUB Rd, Rn, #imm 一条指令, 零运行时开销.
 *
 * 注: 这里是 ch01-ch10 沿用的函数式包装教学简化版. ch15 (Platform 层)
 * 会重构成 ops 表 (虚函数表) 形式, 和工业代码对齐.
 *
 * 见 ch13 § 13.9 在 STM32 上长什么样.
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
