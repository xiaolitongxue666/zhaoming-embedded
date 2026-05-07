/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - 同一份 led_init / led_on 在 STM32 上长什么样
 *
 * 这是片段, 不是完整工程. 完整 STM32 工程见附录 B.
 *
 * 关键观察: led.h / led.c / main.c 一字不改. 字段在 led.h 公开 (标了
 * "private" 注释), update_hardware / brightness_valid 在 led.c 里加了
 * static. 变化的只是这个 platform_*.c 文件. 这是平台抽象层最直接的威力.
 *
 * pin 编码沿用 ch01 § 1.x 的 PIN_NUM('A', 13) 风格, port 信息藏在 pin
 * 编码里. 后续每一章 platform-mcu/stm32/ 都用同一套.
 */

#include "platform.h"
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
 * 应用层调用方式 (和 PC 版完全一样):
 *
 *   struct led red;
 *   led_init(&red, PIN_NUM('A', 13));    PA.13
 *   led_on(&red);
 *   // red.pin = 999;          软 private 编译能过, 靠纪律拦
 *   // update_hardware(&red);  硬 private 链接器拦 (static)
 *   led_deinit(&red);
 *
 * 软 private + 硬 private 在 ARM Cortex-M 上和 x86 上行为完全一致.
 * 都是工程纪律 + 链接期 static 锁, 跨平台规则一致.
 */
