/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - 父类统一接口 led_on 落到 STM32 上的样子
 *
 * 父类 led_on / led_off 写在 led.c, 子类实现 (gpio_on / pwm_on / i2c_on)
 * 走 platform_gpio_xxx 封装函数. STM32 上这一层落到 HAL_GPIO_xxx,
 * 应用层 / 父类 / 子类 / board_init.c 一字不改.
 *
 * 跟 pc/ 唯一的差别就在这个文件: 把 printf 模拟换成真实 HAL 操作.
 *
 * PWM 子类在真实硬件上用 HAL_TIM_PWM_Start + __HAL_TIM_SET_COMPARE,
 * I2C 子类用 HAL_I2C_Master_Transmit. 这两路不在本片段里, 见附录 B
 * 完整工程.
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
