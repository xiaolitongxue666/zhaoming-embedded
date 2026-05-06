/* SPDX-License-Identifier: MIT */
/*
 * hal_gpio.c - 模拟 HAL 库 GPIO 实现（后厨）
 *
 * .c 文件结构对应 ch04 数据归位：
 *   1. 模块共享数据：g_gpioa_regs / g_gpiob_regs / g_gpioc_regs（在
 *      真实硬件上对应硅片上的寄存器，在 PC 上用全局变量模拟）
 *   2. file-private 工具函数：get_pin_number / set_2bit_field 等加 static
 *   3. 公共函数：HAL_GPIO_Init / HAL_GPIO_WritePin 等
 *
 * 真实 stm32h7xx_hal_gpio.c 的结构和这里几乎完全一样。
 * ST 的工程师在做你这一章学过的所有事。
 */

#include "hal_gpio.h"
#include <stdio.h>

/* 三个 GPIO 端口实例（PC 上模拟硬件寄存器） */
GPIO_TypeDef g_gpioa_regs;
GPIO_TypeDef g_gpiob_regs;
GPIO_TypeDef g_gpioc_regs;

/* ---- file-private 工具函数（后厨） ---- */

static int get_pin_number(uint16_t pin_mask)
{
	for (int i = 0; i < 16; i++)
		if (pin_mask & (1U << i))
			return i;
	return -1;
}

static void set_2bit_field(uint32_t *reg, int pin_pos, uint32_t value)
{
	uint32_t mask = 0x03U << (pin_pos * 2);
	*reg = (*reg & ~mask) | ((value & 0x03U) << (pin_pos * 2));
}

static void set_1bit_field(uint32_t *reg, int pin_pos, uint32_t value)
{
	uint32_t mask = 1U << pin_pos;
	*reg = (*reg & ~mask) | ((value & 0x01U) << pin_pos);
}

static const char *mode_name(uint32_t mode)
{
	switch (mode) {
	case GPIO_MODE_INPUT:  return "Input";
	case GPIO_MODE_OUTPUT: return "Output";
	case GPIO_MODE_AF:     return "AF";
	case GPIO_MODE_ANALOG: return "Analog";
	default:               return "Unknown";
	}
}

static const char *pull_name(uint32_t pull)
{
	switch (pull) {
	case GPIO_PUPD_NONE: return "No Pull";
	case GPIO_PUPD_UP:   return "Pull-up";
	case GPIO_PUPD_DOWN: return "Pull-down";
	default:             return "Unknown";
	}
}

static const char *speed_name(uint32_t speed)
{
	switch (speed) {
	case GPIO_SPEED_LOW:       return "Low";
	case GPIO_SPEED_MEDIUM:    return "Medium";
	case GPIO_SPEED_HIGH:      return "High";
	case GPIO_SPEED_VERY_HIGH: return "Very High";
	default:                   return "Unknown";
	}
}

/* ---- 公共函数实现（菜单上的菜） ---- */

void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *init)
{
	if (!GPIOx || !init) {
		printf("  [HAL_GPIO] Error: NULL pointer\n");
		return;
	}

	const char *port = gpio_port_name(GPIOx);

	for (int pin = 0; pin < 16; pin++) {
		if (!(init->Pin & (1U << pin)))
			continue;

		set_2bit_field(&GPIOx->MODER, pin, init->Mode);
		set_1bit_field(&GPIOx->OTYPER, pin, init->OType);
		set_2bit_field(&GPIOx->OSPEEDR, pin, init->Speed);
		set_2bit_field(&GPIOx->PUPDR, pin, init->Pull);

		printf("  [HAL_GPIO_Init] %s Pin%d: Mode=%s Speed=%s %s\n",
		       port, pin, mode_name(init->Mode),
		       speed_name(init->Speed), pull_name(init->Pull));
	}
}

void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t pin)
{
	if (!GPIOx)
		return;

	const char *port = gpio_port_name(GPIOx);

	for (int i = 0; i < 16; i++) {
		if (!(pin & (1U << i)))
			continue;

		set_2bit_field(&GPIOx->MODER, i, GPIO_MODE_INPUT);
		set_1bit_field(&GPIOx->OTYPER, i, 0);
		set_2bit_field(&GPIOx->OSPEEDR, i, GPIO_SPEED_LOW);
		set_2bit_field(&GPIOx->PUPDR, i, GPIO_PUPD_NONE);
		GPIOx->ODR &= ~(1U << i);

		printf("  [HAL_GPIO_DeInit] %s Pin%d reset to default\n",
		       port, i);
	}
}

void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t pin, bool value)
{
	if (!GPIOx)
		return;

	/*
	 * 见书 ch05 § 5.8 + § 5.8.5 BSRR / ODR / LCKR 三个寄存器一组对照。
	 *
	 * 真实 STM32 上：
	 *   value=true  -> GPIOx->BSRR = pin;          // 低 16 位置 1，引脚拉高
	 *   value=false -> GPIOx->BSRR = pin << 16;    // 高 16 位置 1，引脚拉低
	 *
	 * 为什么不直接写 ODR? ODR 的 read-modify-write (读 / OR / 写回)
	 * 是三条指令, 中断在中间插一脚改其他引脚状态就乱了。BSRR 设计成
	 * "写 1 才生效, 写 0 无影响", 一条 32 位 store 搞定, 多任务多
	 * 中断同时操作不同引脚时不打架 (atomic single-pin set)。
	 *
	 * 这种"硬件帮你做并发安全"的设计哲学, 在工业代码里到处都是。
	 */
	if (value) {
		GPIOx->BSRR = pin;
		GPIOx->ODR |= pin;
	} else {
		GPIOx->BSRR = (uint32_t)pin << 16;
		GPIOx->ODR &= ~(uint32_t)pin;
	}

	const char *port = gpio_port_name(GPIOx);
	int pin_num = get_pin_number(pin);
	printf("  [HAL_GPIO_WritePin] %s Pin%d -> %s\n",
	       port, pin_num, value ? "HIGH" : "LOW");
}

bool HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t pin)
{
	if (!GPIOx)
		return false;

	bool value = (GPIOx->ODR & pin) != 0;

	const char *port = gpio_port_name(GPIOx);
	int pin_num = get_pin_number(pin);
	printf("  [HAL_GPIO_ReadPin] %s Pin%d <- %s\n",
	       port, pin_num, value ? "HIGH" : "LOW");
	return value;
}

void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t pin)
{
	if (!GPIOx)
		return;

	GPIOx->ODR ^= pin;

	const char *port = gpio_port_name(GPIOx);
	int pin_num = get_pin_number(pin);
	bool new_value = (GPIOx->ODR & pin) != 0;
	printf("  [HAL_GPIO_TogglePin] %s Pin%d -> %s\n",
	       port, pin_num, new_value ? "HIGH" : "LOW");
}
