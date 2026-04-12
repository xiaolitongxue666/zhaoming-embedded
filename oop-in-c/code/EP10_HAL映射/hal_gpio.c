/**
 * @file     hal_gpio.c
 * @brief    模拟HAL库GPIO实现 - "后厨"（static辅助函数，EP07）
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP10 - HAL库几千个函数，就一个套路
 *
 * 这个文件对应EP07学的概念：
 *
 *   .c文件 = 后厨。顾客（调用者）看不到里面怎么做菜，
 *   只能通过菜单（.h文件）点菜。
 *
 *   static函数 = 后厨专用工具。
 *   外部调不到，内部随便用。
 *   和EP07的 update_hardware() 是一个道理。
 *
 *   真实HAL库的.c文件里也有一堆static辅助函数，
 *   你在头文件里永远看不到它们——因为那是后厨的事。
 *
 * 这个文件同时对应EP09学的概念：
 *
 *   g_gpioa_regs / g_gpiob_regs / g_gpioc_regs
 *   是模块级数据（file-scope），定义在.c文件里。
 *   数据有主人，不裸奔。
 */

#include "hal_gpio.h"
#include <stdio.h>
#include <string.h>

/* ================================================================
 *  GPIO端口实例定义 —— 模拟硬件寄存器
 *
 *  真实STM32：这些寄存器在芯片内部，地址由硬件决定。
 *  我们的模拟：用全局变量代替，效果一样——
 *  同一个struct，不同的实例。（EP06多实例）
 * ================================================================ */

GPIO_TypeDef g_gpioa_regs = {0};
GPIO_TypeDef g_gpiob_regs = {0};
GPIO_TypeDef g_gpioc_regs = {0};

/* ================================================================
 *  static辅助函数 —— 后厨专用（EP07）
 *
 *  这些函数外部看不到、调不到。
 *  和EP07里 led.c 中的 static update_hardware() 是一个道理。
 *
 *  真实HAL库的 stm32f4xx_hal_gpio.c 里也有类似的static函数，
 *  比如参数检查、位操作辅助等——后厨工具，顾客无需知道。
 * ================================================================ */

/**
 * @brief  根据引脚位掩码获取引脚编号（0~15）
 * @note   static = 后厨专用，外部调不到
 */
static int get_pin_number(uint16_t pin_mask) {
    for (int i = 0; i < 16; i++) {
        if (pin_mask & (1U << i)) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief  设置寄存器中指定引脚的2-bit字段
 * @note   static = 后厨专用
 *
 * GPIO的MODER/OSPEEDR/PUPDR寄存器都是每2bit控制一个引脚，
 * 这个函数统一处理2-bit字段的写入。
 */
static void set_2bit_field(uint32_t *reg, int pin_pos, uint32_t value) {
    uint32_t mask = 0x03U << (pin_pos * 2);
    *reg = (*reg & ~mask) | ((value & 0x03U) << (pin_pos * 2));
}

/**
 * @brief  设置寄存器中指定引脚的1-bit字段
 * @note   static = 后厨专用
 *
 * GPIO的OTYPER寄存器是每1bit控制一个引脚。
 */
static void set_1bit_field(uint32_t *reg, int pin_pos, uint32_t value) {
    uint32_t mask = 1U << pin_pos;
    *reg = (*reg & ~mask) | ((value & 0x01U) << pin_pos);
}

/**
 * @brief  打印引脚模式的中文描述
 * @note   static = 后厨专用
 */
static const char *mode_name(uint32_t mode) {
    switch (mode) {
    case GPIO_MODE_INPUT:   return "Input";
    case GPIO_MODE_OUTPUT:  return "Output";
    case GPIO_MODE_AF:      return "AF";
    case GPIO_MODE_ANALOG:  return "Analog";
    default:                return "Unknown";
    }
}

/**
 * @brief  打印上下拉配置的中文描述
 * @note   static = 后厨专用
 */
static const char *pull_name(uint32_t pull) {
    switch (pull) {
    case GPIO_PUPD_NONE:    return "No Pull";
    case GPIO_PUPD_UP:      return "Pull-up";
    case GPIO_PUPD_DOWN:    return "Pull-down";
    default:                return "Unknown";
    }
}

/**
 * @brief  打印输出速度的中文描述
 * @note   static = 后厨专用
 */
static const char *speed_name(uint32_t speed) {
    switch (speed) {
    case GPIO_SPEED_LOW:        return "Low";
    case GPIO_SPEED_MEDIUM:     return "Medium";
    case GPIO_SPEED_HIGH:       return "High";
    case GPIO_SPEED_VERY_HIGH:  return "Very High";
    default:                    return "Unknown";
    }
}

/* ================================================================
 *  公共函数实现 —— 菜单上的菜（EP07）
 *
 *  每个函数第一个参数：GPIO_TypeDef *GPIOx —— me指针（EP06）
 *  函数前缀：HAL_GPIO_ —— 类名（EP08）
 * ================================================================ */

/**
 * HAL_GPIO_Init —— 构造函数（EP08）
 *
 * 对比EP08：
 *   led_init(Led_t *me, uint8_t pin)
 *   HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *init)
 *
 *   参数1：me指针（"我操作的是哪个对象"）
 *   参数2：配置信息
 *   套路一模一样。
 */
void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *init) {
    /* 参数检查——和led_init里检查NULL是一个道理 */
    if (GPIOx == NULL || init == NULL) {
        printf("  [HAL_GPIO] Error: NULL pointer!\n");
        return;
    }

    const char *port = gpio_port_name(GPIOx);

    /* 遍历所有引脚，配置被选中的 */
    for (int pin = 0; pin < 16; pin++) {
        if (!(init->Pin & (1U << pin))) {
            continue;   /* 这个引脚没被选中，跳过 */
        }

        /* 设置模式寄存器（MODER）—— 每个引脚占2bit */
        set_2bit_field(&GPIOx->MODER, pin, init->Mode);

        /* 设置输出类型（OTYPER）—— 每个引脚占1bit */
        set_1bit_field(&GPIOx->OTYPER, pin, init->OType);

        /* 设置输出速度（OSPEEDR）—— 每个引脚占2bit */
        set_2bit_field(&GPIOx->OSPEEDR, pin, init->Speed);

        /* 设置上下拉（PUPDR）—— 每个引脚占2bit */
        set_2bit_field(&GPIOx->PUPDR, pin, init->Pull);

        printf("  [HAL_GPIO_Init] %s Pin%d configured: Mode=%s, Speed=%s, %s\n",
               port, pin, mode_name(init->Mode),
               speed_name(init->Speed), pull_name(init->Pull));
    }
}

/**
 * HAL_GPIO_DeInit —— 析构函数（EP08）
 *
 * 对比EP08：
 *   led_deinit(Led_t *me)
 *   HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t pin)
 *
 *   把引脚恢复为默认状态——和led_deinit清除所有状态是一个道理。
 */
void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t pin) {
    if (GPIOx == NULL) {
        printf("  [HAL_GPIO] Error: NULL pointer!\n");
        return;
    }

    const char *port = gpio_port_name(GPIOx);

    for (int i = 0; i < 16; i++) {
        if (!(pin & (1U << i))) {
            continue;
        }

        /* 全部恢复为默认值（输入模式、无上下拉） */
        set_2bit_field(&GPIOx->MODER, i, GPIO_MODE_INPUT);
        set_1bit_field(&GPIOx->OTYPER, i, 0);
        set_2bit_field(&GPIOx->OSPEEDR, i, GPIO_SPEED_LOW);
        set_2bit_field(&GPIOx->PUPDR, i, GPIO_PUPD_NONE);

        /* 清除输出数据 */
        GPIOx->ODR &= ~(1U << i);

        printf("  [HAL_GPIO_DeInit] %s Pin%d reset to default (closed)\n",
               port, i);
    }
}

/**
 * HAL_GPIO_WritePin —— 写引脚
 *
 * 对比EP06-08：
 *   led_on(Led_t *me)      → 操作me指向的LED
 *   HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, ...) → 操作GPIOx指向的端口
 *
 *   me指针决定"操作谁"——EP06就教过。
 */
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t pin, bool value) {
    if (GPIOx == NULL) {
        return;
    }

    const char *port = gpio_port_name(GPIOx);
    int pin_num = get_pin_number(pin);

    if (value) {
        /* 置位：通过BSRR低16位置1 */
        GPIOx->BSRR = (uint32_t)pin;
        GPIOx->ODR |= (uint32_t)pin;
    } else {
        /* 复位：通过BSRR高16位置1 */
        GPIOx->BSRR = (uint32_t)pin << 16;
        GPIOx->ODR &= ~(uint32_t)pin;
    }

    printf("  [HAL_GPIO_WritePin] %s Pin%d -> %s\n",
           port, pin_num, value ? "HIGH(1)" : "LOW(0)");
}

/**
 * HAL_GPIO_ReadPin —— 读引脚
 */
bool HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t pin) {
    if (GPIOx == NULL) {
        return false;
    }

    /* 读ODR寄存器对应的bit（模拟读取引脚电平） */
    bool value = (GPIOx->ODR & (uint32_t)pin) != 0;

    const char *port = gpio_port_name(GPIOx);
    int pin_num = get_pin_number(pin);
    printf("  [HAL_GPIO_ReadPin] %s Pin%d <- %s\n",
           port, pin_num, value ? "HIGH(1)" : "LOW(0)");

    return value;
}

/**
 * HAL_GPIO_TogglePin —— 翻转引脚
 *
 * 对比EP08：
 *   led_toggle(Led_t *me)
 *   HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t pin)
 *
 *   操作对象不同（LED vs GPIO端口），套路完全一样。
 */
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t pin) {
    if (GPIOx == NULL) {
        return;
    }

    /* 翻转ODR对应的bit */
    GPIOx->ODR ^= (uint32_t)pin;

    const char *port = gpio_port_name(GPIOx);
    int pin_num = get_pin_number(pin);
    bool new_value = (GPIOx->ODR & (uint32_t)pin) != 0;
    printf("  [HAL_GPIO_TogglePin] %s Pin%d toggled -> %s\n",
           port, pin_num, new_value ? "HIGH(1)" : "LOW(0)");
}
