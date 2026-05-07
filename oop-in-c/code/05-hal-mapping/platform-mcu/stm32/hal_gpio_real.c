/* SPDX-License-Identifier: MIT */
/*
 * hal_gpio_real.c - 真实 STM32 HAL 库的 GPIO 函数对照
 *
 * 这是片段，用来对比"教学版 vs 真实 HAL"。完整 STM32 工程见附录 B。
 *
 * 真实文件：stm32h7xx_hal_gpio.c
 * 来源：ST Microelectronics CubeH7 Repository
 * 版权：ST 持有，仅作教学引用
 *
 * 关键观察：
 *   1. 函数签名和我们的教学版几乎一字不差
 *   2. ST 工程师在做你这一章学过的所有事
 */

#include "stm32h7xx_hal.h"

/*
 * HAL_GPIO_Init 真实实现节选（简化展示，省略 IO 复用 / EXTI 配置）：
 *
 * void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init)
 * {
 *     uint32_t position = 0x00U;
 *     uint32_t iocurrent;
 *     uint32_t temp;
 *
 *     while (((GPIO_Init->Pin) >> position) != 0U) {
 *         iocurrent = (GPIO_Init->Pin) & (1UL << position);
 *
 *         if (iocurrent != 0U) {
 *             // ---- MODER 寄存器 ----
 *             temp = GPIOx->MODER;
 *             temp &= ~(GPIO_MODER_MODE0 << (position * 2U));
 *             temp |= ((GPIO_Init->Mode & GPIO_MODE) << (position * 2U));
 *             GPIOx->MODER = temp;
 *
 *             // ---- OTYPER 寄存器 ----
 *             temp = GPIOx->OTYPER;
 *             temp &= ~(GPIO_OTYPER_OT0 << position);
 *             temp |= (((GPIO_Init->Mode & GPIO_OUTPUT_TYPE) >> 4U) << position);
 *             GPIOx->OTYPER = temp;
 *
 *             // ---- OSPEEDR 寄存器 ----
 *             temp = GPIOx->OSPEEDR;
 *             temp &= ~(GPIO_OSPEEDR_OSPEED0 << (position * 2U));
 *             temp |= (GPIO_Init->Speed << (position * 2U));
 *             GPIOx->OSPEEDR = temp;
 *
 *             // ---- PUPDR 寄存器 ----
 *             temp = GPIOx->PUPDR;
 *             temp &= ~(GPIO_PUPDR_PUPD0 << (position * 2U));
 *             temp |= ((GPIO_Init->Pull) << (position * 2U));
 *             GPIOx->PUPDR = temp;
 *         }
 *         position++;
 *     }
 * }
 *
 * 这就是 ch04 的 set_2bit_field / set_1bit_field 工业级版本，
 * ST 工程师为了性能展开了循环但思路一模一样。
 */

void HAL_GPIO_WritePin_demo(void)
{
	GPIO_InitTypeDef cfg = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE();

	cfg.Pin   = GPIO_PIN_5;
	cfg.Mode  = GPIO_MODE_OUTPUT_PP;
	cfg.Pull  = GPIO_NOPULL;
	cfg.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &cfg);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/*
 * HAL_GPIO_WritePin 真实实现：
 *
 * void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
 *                        GPIO_PinState PinState)
 * {
 *     if (PinState != GPIO_PIN_RESET)
 *         GPIOx->BSRR = (uint32_t)GPIO_Pin;
 *     else
 *         GPIOx->BSRR = (uint32_t)GPIO_Pin << GPIO_NUMBER;  // 16
 * }
 *
 * 一行代码搞定，BSRR 寄存器的 atomic 写入直接改硬件位。
 * 这就是教学版 hal_gpio.c 里 HAL_GPIO_WritePin 的完整逻辑，
 * 只是真实版没有 printf。
 */
