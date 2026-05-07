/* SPDX-License-Identifier: MIT */
/*
 * pin_board.c - STM32 board-level PIN driver implementation (HAL based).
 *
 * STM32 真机 pin 子类: 实现 _stm32_pin_ops 这一组函数, 启动期 INIT_BOARD_EXPORT
 * 自动注册到 platform_pin 框架. 上层 driver 调 platform_pin_xxx 时通过框架
 * 内部 ops 指针 dispatch 到这一份.
 *
 * 跨芯片移植 (STM32F4 -> H7 / F1 / G0): 复制这一份, 改 #include 头文件名,
 * port 解码表 / IRQ 映射可能微调.
 */

#include <string.h>

#include "platform/platform_module_export.h"
#include "platform/platform_pin.h"
#include "stm32f4xx_hal.h"

/* PIN 编码: 高 4 位放 port, 低 4 位放 num. PA.0 -> 0x00, PD.12 -> 0x3C. */
#define PIN_NUM(port, no)        (((((port) & 0xFu) << 4) | ((no) & 0xFu)))
#define PIN_PORT(pin)            ((uint8_t)(((pin) >> 4) & 0xFu))
#define PIN_NO(pin)              ((uint8_t)((pin) & 0xFu))
#define PIN_STPORT(pin)          ((GPIO_TypeDef *)(GPIOA_BASE + (0x400u * PIN_PORT(pin))))
#define PIN_STPIN(pin)           ((uint16_t)(1u << PIN_NO(pin)))

#if defined(GPIOI)
#define __STM32_PORT_MAX         9u
#elif defined(GPIOH)
#define __STM32_PORT_MAX         8u
#elif defined(GPIOG)
#define __STM32_PORT_MAX         7u
#elif defined(GPIOF)
#define __STM32_PORT_MAX         6u
#elif defined(GPIOE)
#define __STM32_PORT_MAX         5u
#elif defined(GPIOD)
#define __STM32_PORT_MAX         4u
#elif defined(GPIOC)
#define __STM32_PORT_MAX         3u
#elif defined(GPIOB)
#define __STM32_PORT_MAX         2u
#elif defined(GPIOA)
#define __STM32_PORT_MAX         1u
#else
#define __STM32_PORT_MAX         0u
#error Unsupported STM32 GPIO peripheral.
#endif

/* "PA.5" / "PD.12" / "PI.14" -> pin number, 或 PLATFORM_EINVAL */
static int32_t _stm32_pin_get(const char *name)
{
	int32_t ret = PLATFORM_EINVAL;
	int     hw_port_num;
	int     hw_pin_num = 0;
	int     i;
	int     name_len;

	name_len = (int)strlen(name);

	if ((name_len < 4) || (name_len >= 6)) {
		goto exit;
	}
	if ((name[0] != 'P') || (name[2] != '.')) {
		goto exit;
	}
	if ((name[1] < 'A') || (name[1] > 'Z')) {
		goto exit;
	}
	hw_port_num = (int)(name[1] - 'A');

	for (i = 3; i < name_len; i++) {
		hw_pin_num *= 10;
		hw_pin_num += name[i] - '0';
	}

	ret = PIN_NUM(hw_port_num, hw_pin_num);

exit:
	return ret;
}

static void _stm32_pin_mode(int32_t pin, int32_t mode)
{
	GPIO_InitTypeDef gi = {0};

	if (PIN_PORT(pin) >= __STM32_PORT_MAX) {
		goto exit;
	}

	gi.Pin   = PIN_STPIN(pin);
	gi.Speed = GPIO_SPEED_FREQ_HIGH;

	switch (mode) {
	case PIN_MODE_OUTPUT:
		gi.Mode = GPIO_MODE_OUTPUT_PP;
		gi.Pull = GPIO_NOPULL;
		break;
	case PIN_MODE_INPUT:
		gi.Mode = GPIO_MODE_INPUT;
		gi.Pull = GPIO_NOPULL;
		break;
	case PIN_MODE_INPUT_PULLUP:
		gi.Mode = GPIO_MODE_INPUT;
		gi.Pull = GPIO_PULLUP;
		break;
	case PIN_MODE_INPUT_PULLDOWN:
		gi.Mode = GPIO_MODE_INPUT;
		gi.Pull = GPIO_PULLDOWN;
		break;
	case PIN_MODE_OUTPUT_OD:
		gi.Mode = GPIO_MODE_OUTPUT_OD;
		gi.Pull = GPIO_NOPULL;
		break;
	default:
		goto exit;
	}

	HAL_GPIO_Init(PIN_STPORT(pin), &gi);

exit:
	return;
}

static void _stm32_pin_write(int32_t pin, int32_t value)
{
	if (PIN_PORT(pin) >= __STM32_PORT_MAX) {
		goto exit;
	}
	HAL_GPIO_WritePin(PIN_STPORT(pin), PIN_STPIN(pin),
	                  (GPIO_PinState)value);

exit:
	return;
}

static int32_t _stm32_pin_read(int32_t pin)
{
	int32_t ret = PIN_LOW;

	if (PIN_PORT(pin) >= __STM32_PORT_MAX) {
		goto exit;
	}
	ret = (HAL_GPIO_ReadPin(PIN_STPORT(pin), PIN_STPIN(pin))
	            == GPIO_PIN_SET) ? PIN_HIGH : PIN_LOW;

exit:
	return ret;
}

static const struct platform_pin_ops _stm32_pin_ops = {
	.mode       = _stm32_pin_mode,
	.write      = _stm32_pin_write,
	.read       = _stm32_pin_read,
	.attach_irq = NULL,    /* 教学版未实现 IRQ, 工业版补 */
	.detach_irq = NULL,
	.irq_enable = NULL,
	.get        = _stm32_pin_get,
};

static void _pin_board_init(void)
{
#if defined(__HAL_RCC_GPIOA_CLK_ENABLE)
	__HAL_RCC_GPIOA_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_GPIOB_CLK_ENABLE)
	__HAL_RCC_GPIOB_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_GPIOC_CLK_ENABLE)
	__HAL_RCC_GPIOC_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_GPIOD_CLK_ENABLE)
	__HAL_RCC_GPIOD_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_GPIOE_CLK_ENABLE)
	__HAL_RCC_GPIOE_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_GPIOF_CLK_ENABLE)
	__HAL_RCC_GPIOF_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_GPIOG_CLK_ENABLE)
	__HAL_RCC_GPIOG_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_GPIOH_CLK_ENABLE)
	__HAL_RCC_GPIOH_CLK_ENABLE();
#endif
#if defined(__HAL_RCC_GPIOI_CLK_ENABLE)
	__HAL_RCC_GPIOI_CLK_ENABLE();
#endif

	(void)platform_pin_register(&_stm32_pin_ops);
}
INIT_BOARD_EXPORT(_pin_board_init);
