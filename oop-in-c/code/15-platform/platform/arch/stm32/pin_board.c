/* SPDX-License-Identifier: MIT */
/*
 * platform/arch/stm32/pin_board.c - STM32F4 端 PIN 后端实现.
 *
 * "认识 STM32 GPIO 寄存器"的唯一一份文件. 上层 drivers/led/led_gpio 调
 * platform_pin_xxx ops 表层接口下来, 落到这里之后才碰 GPIO_TypeDef *.
 *
 * 跨 MCU (STM32 -> NXP) 的"换 6 份之一": 这一份 pin_board.c 加上同目录的
 * pwm_board.c (TIM PWM 后端) + i2c_board.c (I2C bus 后端) 共三份, 是这家
 * MCU 的 platform 实现层. 上面所有文件 (drivers/led 下 4 对 .h/.c,
 * platform 接口层 .h+.c) 字节级不动. 这是 ch15 § 15.11.5 "STM32 vs NXP"
 * 的代码兑现点.
 *
 * 工程假设: STM32CubeMX 已经配好 GPIO mux + clock, 真实工程把 #include 的
 * stm32f4xx_hal.h 换成对应工程的 HAL 头, 解码表也按板子上的实际 port 数量
 * 重写.
 */

#include "platform/platform_pin.h"
#include "stm32f4xx_hal.h"

#include <string.h>

/* PIN 编码: 高 4 位 port, 低 4 位 num. PA.0 -> 0x00, PD.12 -> 0x3C. */
#define PIN_PORT(pin)        ((uint8_t)(((pin) >> 4) & 0x0F))
#define PIN_OFFSET(pin)      ((uint8_t)((pin) & 0x0F))
#define PIN_MASK(pin)        ((uint16_t)(1U << PIN_OFFSET(pin)))

/* port 解码表: 0..8 -> GPIOA..GPIOI. */
static GPIO_TypeDef *const _gpio_table[] = {
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

#define _GPIO_TABLE_SIZE  (sizeof(_gpio_table) / sizeof(_gpio_table[0]))

static int32_t _stm32_pin_get(const char *name)
{
	int port_num;
	int pin_num = 0;
	int len;
	int i;

	if (!name)
		return -1;
	len = (int)strlen(name);
	if (len < 4 || len >= 6)
		return -1;
	if (name[0] != 'P' || name[2] != '.')
		return -1;
	if (name[1] < 'A' || name[1] > 'Z')
		return -1;

	port_num = name[1] - 'A';
	for (i = 3; i < len; i++)
		pin_num = pin_num * 10 + (name[i] - '0');

	return ((port_num & 0x0F) << 4) | (pin_num & 0x0F);
}

static void _stm32_pin_mode(int32_t pin, int32_t mode)
{
	GPIO_TypeDef    *port = _gpio_table[PIN_PORT(pin)];
	GPIO_InitTypeDef cfg  = {0};

	if (PIN_PORT(pin) >= _GPIO_TABLE_SIZE || port == NULL)
		return;

	cfg.Pin   = PIN_MASK(pin);
	cfg.Speed = GPIO_SPEED_FREQ_LOW;
	cfg.Pull  = GPIO_NOPULL;
	switch (mode) {
	case PIN_MODE_OUTPUT:        cfg.Mode = GPIO_MODE_OUTPUT_PP; break;
	case PIN_MODE_INPUT:         cfg.Mode = GPIO_MODE_INPUT;     break;
	case PIN_MODE_INPUT_PULLUP:  cfg.Mode = GPIO_MODE_INPUT;
	                             cfg.Pull = GPIO_PULLUP;         break;
	default: return;
	}
	HAL_GPIO_Init(port, &cfg);
}

static void _stm32_pin_write(int32_t pin, int32_t value)
{
	GPIO_TypeDef *port = _gpio_table[PIN_PORT(pin)];

	if (PIN_PORT(pin) >= _GPIO_TABLE_SIZE || port == NULL)
		return;
	HAL_GPIO_WritePin(port, PIN_MASK(pin),
	                  value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static int32_t _stm32_pin_read(int32_t pin)
{
	GPIO_TypeDef *port = _gpio_table[PIN_PORT(pin)];

	if (PIN_PORT(pin) >= _GPIO_TABLE_SIZE || port == NULL)
		return PIN_LOW;
	return (HAL_GPIO_ReadPin(port, PIN_MASK(pin)) == GPIO_PIN_SET)
	       ? PIN_HIGH : PIN_LOW;
}

static const struct platform_pin_ops _stm32_pin_ops = {
	.mode  = _stm32_pin_mode,
	.write = _stm32_pin_write,
	.read  = _stm32_pin_read,
	.get   = _stm32_pin_get,
};

/* 启动期由 board_init 调一次. 真机工程在 main / SystemInit 之后调 */
void platform_hw_pin_init(void)
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
	(void)platform_pin_register(&_stm32_pin_ops);
}
