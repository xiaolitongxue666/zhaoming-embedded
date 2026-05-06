/* SPDX-License-Identifier: MIT */
/*
 * gpio_typedef.h - 模拟 STM32 GPIO 寄存器结构体与端口实例定义
 *
 * 这个文件不是真实的 STM32 HAL，是教学用的迷你版本，在 PC 上能跑。
 * 真实 STM32H7 的 stm32h7xx.h 里 GPIO_TypeDef 结构体长得几乎一模一样，
 * 字段顺序和名字都对应同一份硅片版图。
 *
 * 三个观察点：
 *   1. GPIO_TypeDef = struct 把寄存器打包（ch01 的 struct）
 *   2. GPIOA / GPIOB / GPIOC = 同一个 struct 的多个实例（ch01 的 me 指针）
 *   3. 字段名 / 函数前缀 / Init 命名 / static 工具函数全是 ch01-ch04 学过的。
 */

#ifndef GPIO_TYPEDEF_H
#define GPIO_TYPEDEF_H

#include <stdint.h>
#include <stdbool.h>

/*
 * GPIO 寄存器结构体
 * 真实 STM32H7 的 GPIO_TypeDef 字段顺序也是这一套（位置和真实 SoC
 * 的硬件版图严格一致，字段相对偏移是固定的）。
 */
typedef struct {
	uint32_t MODER;     /* 模式寄存器：每 2 bit 控制一个引脚的模式 */
	uint32_t OTYPER;    /* 输出类型寄存器：推挽 / 开漏 */
	uint32_t OSPEEDR;   /* 输出速度寄存器 */
	uint32_t PUPDR;     /* 上下拉寄存器 */
	uint32_t IDR;       /* 输入数据寄存器（只读） */
	uint32_t ODR;       /* 输出数据寄存器 */
	uint32_t BSRR;      /* 置位 / 复位寄存器 */
	uint32_t LCKR;      /* 锁定寄存器 */
} GPIO_TypeDef;

/* GPIO 引脚模式定义（HAL 真实命名） */
#define GPIO_MODE_INPUT      0x00U
#define GPIO_MODE_OUTPUT     0x01U
#define GPIO_MODE_AF         0x02U
#define GPIO_MODE_ANALOG     0x03U

/* 输出类型 */
#define GPIO_OTYPE_PP        0x00U
#define GPIO_OTYPE_OD        0x01U

/* 输出速度 */
#define GPIO_SPEED_LOW       0x00U
#define GPIO_SPEED_MEDIUM    0x01U
#define GPIO_SPEED_HIGH      0x02U
#define GPIO_SPEED_VERY_HIGH 0x03U

/* 上下拉 */
#define GPIO_PUPD_NONE       0x00U
#define GPIO_PUPD_UP         0x01U
#define GPIO_PUPD_DOWN       0x02U

/*
 * 引脚编号宏：每个引脚对应一个 bit 位（HAL 真实命名）
 */
#define GPIO_PIN_0           ((uint16_t)0x0001)
#define GPIO_PIN_1           ((uint16_t)0x0002)
#define GPIO_PIN_2           ((uint16_t)0x0004)
#define GPIO_PIN_3           ((uint16_t)0x0008)
#define GPIO_PIN_4           ((uint16_t)0x0010)
#define GPIO_PIN_5           ((uint16_t)0x0020)
#define GPIO_PIN_6           ((uint16_t)0x0040)
#define GPIO_PIN_7           ((uint16_t)0x0080)
#define GPIO_PIN_8           ((uint16_t)0x0100)
#define GPIO_PIN_9           ((uint16_t)0x0200)
#define GPIO_PIN_10          ((uint16_t)0x0400)
#define GPIO_PIN_11          ((uint16_t)0x0800)
#define GPIO_PIN_12          ((uint16_t)0x1000)
#define GPIO_PIN_13          ((uint16_t)0x2000)
#define GPIO_PIN_14          ((uint16_t)0x4000)
#define GPIO_PIN_15          ((uint16_t)0x8000)

/* GPIO 初始化配置结构体（HAL 真实命名） */
typedef struct {
	uint32_t Pin;       /* 要配置的引脚，可用 | 组合多个 */
	uint32_t Mode;      /* 引脚模式 */
	uint32_t Pull;      /* 上下拉配置 */
	uint32_t Speed;     /* 输出速度 */
	uint32_t OType;     /* 输出类型 */
} GPIO_InitTypeDef;

/*
 * 三个 GPIO 端口实例：同一个 struct，不同的"地址"
 *
 * 真实 STM32H7：
 *   #define GPIOA  ((GPIO_TypeDef *)0x58020000UL)
 *   #define GPIOB  ((GPIO_TypeDef *)0x58020400UL)
 * 硬件寄存器映射到不同的物理地址。
 *
 * 我们的模拟：用全局变量代替硬件地址，效果一样：
 *   同一个 struct 定义，多个独立实例。
 */
extern GPIO_TypeDef g_gpioa_regs;
extern GPIO_TypeDef g_gpiob_regs;
extern GPIO_TypeDef g_gpioc_regs;

#define GPIOA   (&g_gpioa_regs)
#define GPIOB   (&g_gpiob_regs)
#define GPIOC   (&g_gpioc_regs)

/* 端口名称辅助（仅用于 PC 模拟版打印） */
static inline const char *gpio_port_name(const GPIO_TypeDef *port)
{
	if (port == GPIOA)
		return "GPIOA";
	if (port == GPIOB)
		return "GPIOB";
	if (port == GPIOC)
		return "GPIOC";
	return "GPIO?";
}

#endif /* GPIO_TYPEDEF_H */
