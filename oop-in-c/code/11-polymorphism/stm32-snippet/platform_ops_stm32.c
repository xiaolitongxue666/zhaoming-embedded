/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_stm32.c - 真实 STM32 上的 platform_ops 实例
 *
 * 这一份只导出 const struct platform_ops platform_stm32. 启动期 main
 * 调 platform_select(&platform_stm32) 把 platform 层内部当前指针指向它.
 * 之后驱动层调 platform_gpio_xxx 封装函数, platform 层内部 dispatch 到
 * 这一份的具体实现.
 *
 * 跟 pc/platform_ops_pc.c 形态一致, 只是底下从 printf 换成 HAL_GPIO_xxx.
 * 应用层 / led 层一字不改.
 */

#include "platform_ops.h"
#include "stm32f4xx_hal.h"

static void stm32_gpio_init(uint8_t pin, uint8_t mode)
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

static void stm32_gpio_deinit(uint8_t pin)
{
	HAL_GPIO_DeInit(GPIOA, (uint16_t)(1U << pin));
}

static void stm32_gpio_write(uint8_t pin, bool value)
{
	HAL_GPIO_WritePin(GPIOA, (uint16_t)(1U << pin),
	                  value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static bool stm32_gpio_read(uint8_t pin)
{
	return HAL_GPIO_ReadPin(GPIOA, (uint16_t)(1U << pin)) == GPIO_PIN_SET;
}

const struct platform_ops platform_stm32 = {
	.name        = "STM32",
	.gpio_init   = stm32_gpio_init,
	.gpio_deinit = stm32_gpio_deinit,
	.gpio_write  = stm32_gpio_write,
	.gpio_read   = stm32_gpio_read,
};

/*
 * 启动时:
 *
 *   int main(void)
 *   {
 *       HAL_Init();
 *       SystemClock_Config();
 *       MX_GPIO_Init();
 *
 *       platform_select(&platform_stm32);
 *       // 之后 led / motor / eeprom 全部走 platform_gpio_xxx 封装函数
 *
 *       struct led_gpio red_led;
 *       led_gpio_init(&red_led, "red", 13);
 *       led_on(&red_led.base);
 *       ...
 *   }
 *
 * 跨芯片移植: 加一个 platform_stm32h7 / platform_esp32 实例, main()
 * 启动时换一行 platform_select(...), 上层全部一字不改. 第 15 章会展开.
 */
