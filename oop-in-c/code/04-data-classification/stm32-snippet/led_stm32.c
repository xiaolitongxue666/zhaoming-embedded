/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - 数据归位完成后，LED 在 STM32 上长什么样
 *
 * 这是片段，不是完整工程。完整 STM32 工程见附录 B。
 *
 * 关键观察：led.h / led.c / main.c 一字不改。静态对象池
 *   static struct led led_pool[8];
 * 落到 STM32 的 .bss 段（清零），上电时编译器自动初始化为 0。
 * 整个程序运行期间这块内存固定，零堆碎片，O(1) 分配 / 释放。
 *
 * 这是工业级 MCU 代码的标准做法，本节 platform 只演示 GPIO 胶水，
 * 完整的对象池机制和应用层是一个 .c 文件就跑通的。
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
 *   struct led *red = led_acquire(13);
 *   led_on(red);
 *   led_release(red);
 *
 * 内存分配在 led.c 内的 led_pool[8]，落 .bss 段，
 * 不走堆。MCU 友好。
 */
