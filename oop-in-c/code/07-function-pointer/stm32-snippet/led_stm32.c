/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - 函数指针变量在 STM32 上的样子
 *
 * 本章主线只演示独立函数指针变量本身 (见 ch07 § 7.4),
 * 没有把指针塞进任何 struct, 也不依赖 LED 结构.
 *
 * STM32 上 gpio_on / gpio_off 的实现就是 ch01 那一套 HAL 调用,
 * 没有引入新的胶水. 把 gpio_on 的地址存进 fp, 通过 fp 调用,
 * 跑出来的指令序列和直接调 gpio_on 几乎一样, 多一次 LDR (从 fp
 * 取地址) + 一次间接 BLX (跳到那个地址).
 */

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

/* GPIO 风格: 拉高/拉低 GPIOA 上某个引脚 */
void gpio_on(uint8_t pin)
{
	HAL_GPIO_WritePin(GPIOA, (uint16_t)(1U << pin), GPIO_PIN_SET);
}

void gpio_off(uint8_t pin)
{
	HAL_GPIO_WritePin(GPIOA, (uint16_t)(1U << pin), GPIO_PIN_RESET);
}

/*
 * 调用方 (节选):
 *
 *   void (*fp)(uint8_t);
 *   fp = gpio_on;
 *   fp(13);                 // 实际调 gpio_on(13), GPIOA Pin13 拉高
 *
 *   fp = gpio_off;
 *   fp(13);                 // 同一行 fp 调用, 这次拨通 gpio_off
 *
 * ARM Cortex-M4 上 fp(13) 编译成:
 *   LDR  r3, [fp_addr]      ; r3 = fp 里存的地址
 *   MOV  r0, #13            ; 第一个参数走 r0
 *   BLX  r3                 ; 间接跳转
 *
 * 见 ch07 § 7.6.5 ABI 章节.
 */
