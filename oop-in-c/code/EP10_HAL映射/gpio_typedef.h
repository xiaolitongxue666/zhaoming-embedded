/**
 * @file     gpio_typedef.h
 * @brief    模拟STM32 GPIO寄存器结构体与端口实例定义
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP10 - HAL库几千个函数，就一个套路
 *
 * 这个文件对应EP06学的两个概念：
 *
 *   1. GPIO_TypeDef = struct封装数据（EP06）
 *      把描述一个GPIO端口的所有寄存器打包在一起，
 *      和Led_t打包pin/brightness/is_on是一个道理。
 *
 *   2. GPIOA / GPIOB / GPIOC = 多实例（EP06）
 *      同一个struct定义，不同的"地址"（实例）。
 *      和 red_led / green_led 是一个道理——
 *      同一份代码，不同的数据。
 *
 * 注意：这不是真实的STM32 HAL库，是教学用的迷你版本，
 *       在PC上编译运行，展示HAL库的设计模式。
 */

#ifndef GPIO_TYPEDEF_H
#define GPIO_TYPEDEF_H

#include <stdint.h>
#include <stdbool.h>

/* ================================================================
 *  GPIO寄存器结构体 —— 和真实STM32 HAL库同一个套路
 *
 *  真实HAL库：typedef struct { ... } GPIO_TypeDef;
 *  我们的：   typedef struct { ... } GPIO_TypeDef;
 *
 *  一模一样。struct封装数据，EP06就教过。
 * ================================================================ */

typedef struct {
    uint32_t MODER;     /* 模式寄存器：每2bit控制一个引脚的模式 */
    uint32_t OTYPER;    /* 输出类型寄存器：推挽/开漏 */
    uint32_t OSPEEDR;   /* 输出速度寄存器 */
    uint32_t PUPDR;     /* 上下拉寄存器 */
    uint32_t IDR;       /* 输入数据寄存器（只读） */
    uint32_t ODR;       /* 输出数据寄存器 */
    uint32_t BSRR;      /* 置位/复位寄存器 */
    uint32_t LCKR;      /* 锁定寄存器 */
} GPIO_TypeDef;

/* ================================================================
 *  GPIO引脚模式定义
 *
 *  真实HAL库也这样定义，名字都一样。
 * ================================================================ */

#define GPIO_MODE_INPUT     0x00U   /* 输入模式 */
#define GPIO_MODE_OUTPUT    0x01U   /* 输出模式 */
#define GPIO_MODE_AF        0x02U   /* 复用模式 */
#define GPIO_MODE_ANALOG    0x03U   /* 模拟模式 */

/* 输出类型 */
#define GPIO_OTYPE_PP       0x00U   /* 推挽输出 */
#define GPIO_OTYPE_OD       0x01U   /* 开漏输出 */

/* 输出速度 */
#define GPIO_SPEED_LOW      0x00U   /* 低速 */
#define GPIO_SPEED_MEDIUM   0x01U   /* 中速 */
#define GPIO_SPEED_HIGH     0x02U   /* 高速 */
#define GPIO_SPEED_VERY_HIGH 0x03U  /* 极高速 */

/* 上下拉 */
#define GPIO_PUPD_NONE      0x00U   /* 无上下拉 */
#define GPIO_PUPD_UP        0x01U   /* 上拉 */
#define GPIO_PUPD_DOWN      0x02U   /* 下拉 */

/* ================================================================
 *  引脚编号宏 —— 每个引脚对应一个bit位
 *
 *  真实HAL库：#define GPIO_PIN_0  ((uint16_t)0x0001)
 *  我们的：   #define GPIO_PIN_0  ((uint16_t)0x0001)
 *
 *  一模一样。
 * ================================================================ */

#define GPIO_PIN_0      ((uint16_t)0x0001)
#define GPIO_PIN_1      ((uint16_t)0x0002)
#define GPIO_PIN_2      ((uint16_t)0x0004)
#define GPIO_PIN_3      ((uint16_t)0x0008)
#define GPIO_PIN_4      ((uint16_t)0x0010)
#define GPIO_PIN_5      ((uint16_t)0x0020)
#define GPIO_PIN_6      ((uint16_t)0x0040)
#define GPIO_PIN_7      ((uint16_t)0x0080)
#define GPIO_PIN_8      ((uint16_t)0x0100)
#define GPIO_PIN_9      ((uint16_t)0x0200)
#define GPIO_PIN_10     ((uint16_t)0x0400)
#define GPIO_PIN_11     ((uint16_t)0x0800)
#define GPIO_PIN_12     ((uint16_t)0x1000)
#define GPIO_PIN_13     ((uint16_t)0x2000)
#define GPIO_PIN_14     ((uint16_t)0x4000)
#define GPIO_PIN_15     ((uint16_t)0x8000)
#define GPIO_PIN_ALL    ((uint16_t)0xFFFF)

/* ================================================================
 *  GPIO初始化配置结构体
 *
 *  真实HAL库：GPIO_InitTypeDef
 *  我们的：   GPIO_InitTypeDef
 *
 *  名字一样，作用一样——把初始化参数打包传给Init函数。
 * ================================================================ */

typedef struct {
    uint32_t Pin;       /* 要配置的引脚，可用 | 组合多个 */
    uint32_t Mode;      /* 引脚模式：输入/输出/复用/模拟 */
    uint32_t Pull;      /* 上下拉配置 */
    uint32_t Speed;     /* 输出速度 */
    uint32_t OType;     /* 输出类型：推挽/开漏 */
} GPIO_InitTypeDef;

/* ================================================================
 *  三个GPIO端口实例 —— 同一个struct，不同的"地址"
 *
 *  真实STM32：
 *    #define GPIOA  ((GPIO_TypeDef *)0x40020000UL)
 *    #define GPIOB  ((GPIO_TypeDef *)0x40020400UL)
 *    硬件寄存器映射到不同的内存地址。
 *
 *  我们的模拟：
 *    用全局变量代替硬件地址，效果一样——
 *    同一个struct定义，不同的实例。
 *
 *  这和EP06的 red_led / green_led 是一个道理：
 *    Led_t red_led;    Led_t green_led;    ← 两个实例
 *    GPIO_TypeDef A;   GPIO_TypeDef B;     ← 两个实例
 *    同一份代码，不同的数据。
 * ================================================================ */

extern GPIO_TypeDef g_gpioa_regs;
extern GPIO_TypeDef g_gpiob_regs;
extern GPIO_TypeDef g_gpioc_regs;

#define GPIOA   (&g_gpioa_regs)
#define GPIOB   (&g_gpiob_regs)
#define GPIOC   (&g_gpioc_regs)

/* 端口名称辅助函数（用于打印） */
static inline const char *gpio_port_name(const GPIO_TypeDef *port) {
    if (port == GPIOA) return "GPIOA";
    if (port == GPIOB) return "GPIOB";
    if (port == GPIOC) return "GPIOC";
    return "GPIO?";
}

#endif /* GPIO_TYPEDEF_H */
