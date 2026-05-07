/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - 共性提取后, STM32 端长什么样
 *
 * 这是片段, 不是完整工程. 完整工程见附录 B.
 *
 * 关键观察: 把 pin 放到子类 led_gpio, channel 放到 led_pwm,
 * STM32 端实现一行不变, led.c / motor.c / main.c 也一字不改.
 * 平台胶水 platform_* 还是 ch01-ch05 的函数式形态.
 *
 * pin 编码沿用 ch01 § 1.x 的 PIN_NUM('A', 13) 风格.
 */

#include "led_gpio.h"
#include "led_pwm.h"
#include "stm32f4xx_hal.h"

#define PIN_PORT_IDX(pin)     (((pin) >> 4) & 0x0F)
#define PIN_NO(pin)           ((pin) & 0x0F)
#define PIN_MASK(pin)         (1U << PIN_NO(pin))

static GPIO_TypeDef * const _gpio_table[] = {
	GPIOA, GPIOB, GPIOC, GPIOD, GPIOE,
#if defined(GPIOF)
	GPIOF,
#else
	NULL,
#endif
#if defined(GPIOG)
	GPIOG,
#else
	NULL,
#endif
#if defined(GPIOH)
	GPIOH,
#else
	NULL,
#endif
#if defined(GPIOI)
	GPIOI,
#else
	NULL,
#endif
};

#define PIN_PORT(pin)    (_gpio_table[PIN_PORT_IDX(pin)])

static void _enable_port_clock(uint8_t pin)
{
	switch (PIN_PORT_IDX(pin)) {
	case 0: __HAL_RCC_GPIOA_CLK_ENABLE(); break;
	case 1: __HAL_RCC_GPIOB_CLK_ENABLE(); break;
	case 2: __HAL_RCC_GPIOC_CLK_ENABLE(); break;
	case 3: __HAL_RCC_GPIOD_CLK_ENABLE(); break;
	case 4: __HAL_RCC_GPIOE_CLK_ENABLE(); break;
#if defined(__HAL_RCC_GPIOF_CLK_ENABLE)
	case 5: __HAL_RCC_GPIOF_CLK_ENABLE(); break;
#endif
#if defined(__HAL_RCC_GPIOG_CLK_ENABLE)
	case 6: __HAL_RCC_GPIOG_CLK_ENABLE(); break;
#endif
#if defined(__HAL_RCC_GPIOH_CLK_ENABLE)
	case 7: __HAL_RCC_GPIOH_CLK_ENABLE(); break;
#endif
#if defined(__HAL_RCC_GPIOI_CLK_ENABLE)
	case 8: __HAL_RCC_GPIOI_CLK_ENABLE(); break;
#endif
	default: break;
	}
}

void platform_gpio_init(uint8_t pin, uint8_t mode)
{
	GPIO_InitTypeDef cfg = {0};

	_enable_port_clock(pin);

	cfg.Pin   = PIN_MASK(pin);
	cfg.Mode  = (mode == GPIO_MODE_OUTPUT) ?
	            GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
	cfg.Pull  = GPIO_NOPULL;
	cfg.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(PIN_PORT(pin), &cfg);
}

void platform_gpio_deinit(uint8_t pin)
{
	HAL_GPIO_DeInit(PIN_PORT(pin), PIN_MASK(pin));
}

void platform_gpio_write(uint8_t pin, bool value)
{
	HAL_GPIO_WritePin(PIN_PORT(pin), PIN_MASK(pin),
	                  value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool platform_gpio_read(uint8_t pin)
{
	return HAL_GPIO_ReadPin(PIN_PORT(pin), PIN_MASK(pin)) == GPIO_PIN_SET;
}

/*
 * 应用层 (节选):
 *
 *   struct led_gpio red_led;
 *   struct led_pwm  blue_led;
 *
 *   led_gpio_init(&red_led, "red", PIN_NUM('A', 13));   PA.13
 *   led_pwm_init(&blue_led, "blue", 1, 70);
 *   led_gpio_on(&red_led);
 *   led_pwm_on(&blue_led);
 *
 * 跨芯片移植: 只重写 platform_gpio_*, 上层一字不改.
 */
