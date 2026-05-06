/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - 函数指针当参数 + 回调注册 在 STM32 上的样子
 *
 * 平台胶水还是 ch01 那套. 函数指针参数 + 回调字段都不影响平台层.
 *
 * 一个真实场景: 主控板每次开关一颗 LED 都要把状态上报到 CAN 总线.
 *   void can_log(struct led *me, bool new_state) {
 *       can_send(0x100 + me->base.pin, new_state ? 1 : 0);
 *   }
 *   led_register_state_cb(&red_led, can_log);
 * 之后 led_on/off 自动把状态发到 CAN, led 模块本身不知道 CAN 存在.
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
