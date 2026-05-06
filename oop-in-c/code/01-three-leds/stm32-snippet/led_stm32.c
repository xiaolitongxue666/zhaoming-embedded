/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - 同一份 led_on / led_off 在 STM32 上长什么样
 *
 * 这是片段，不是完整工程。完整 STM32 工程见附录 B。
 * 用 STM32CubeMX 把对应的 GPIO 配成输出后，下面的实现就能跑。
 *
 * 关键观察：led.h / led.c / main.c 一字不改 —— 上层代码 100% 复用。
 * 变化的只是这个 platform_*.c 文件。这是平台抽象层最直接的威力。
 */

#include "led.h"
#include "stm32f4xx_hal.h"

/*
 * 把 PC 模拟版的 platform.h 接口接到 STM32 HAL 上。
 *
 * 假设硬件上 Pin 13/14/15 接到了 GPIOA 的 PA13/PA14/PA15。
 * 真实工程里这个映射会做成查找表或宏，这里写死最直观。
 */

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
 * 应用层调用方式：
 *
 *   struct led red_led;
 *   led_init(&red_led, 13);
 *   led_on(&red_led);     <-- 和 PC 版完全一样
 *
 * 编译命令（CubeIDE 或命令行 arm-none-eabi-gcc）：
 *
 *   arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -DSTM32F407xx \
 *       -I... -o firmware.elf \
 *       main.c led.c led_stm32.c stm32f4xx_hal_*.c
 *
 * 烧录后真实 LED 会按 main.c 描述的顺序点亮。
 */
