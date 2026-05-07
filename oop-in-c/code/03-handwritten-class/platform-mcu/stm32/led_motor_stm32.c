/* SPDX-License-Identifier: MIT */
/*
 * led_motor_stm32.c - led + motor 的 platform 在 STM32 上长什么样
 *
 * 这是片段, 不是完整工程. 完整 STM32 工程见附录 B.
 *
 * 关键观察: led.h / led.c / motor.h / motor.c / main.c 全部一字不改.
 * 两个模块照样跑, 前缀照样区分得开. 变化的只是这个 platform_*.c 文件.
 *
 * 真实工程里 motor 的 PWM 速度控制会写到 TIM 通道的 CCRx 寄存器,
 * 这里教学简化为 GPIO 高低电平. 第 5 章会展开 HAL 怎么做映射.
 *
 * pin 编码沿用 ch01 § 1.x 的 PIN_NUM('A', 13) 风格, port 信息藏在
 * pin 编码里.
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
 *   struct motor fan;
 *   led_init(&red, PIN_NUM('A', 13));    PA.13
 *   motor_init(&fan, PIN_NUM('A', 5));   PA.5
 *   led_on(&red);
 *   motor_start(&fan);
 *
 * 命名前缀的好处在 STM32 上同样体现:
 * 三家芯片厂的 HAL 库共存 (HAL_GPIO_xxx, BSP_LED_xxx, drv_motor_xxx),
 * 大家不会撞名.
 */
