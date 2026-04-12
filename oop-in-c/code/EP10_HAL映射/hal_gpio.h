/**
 * @file     hal_gpio.h
 * @brief    模拟HAL库GPIO公共接口 - "菜单"（.h声明，EP07）
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP10 - HAL库几千个函数，就一个套路
 *
 * 这个文件对应EP07和EP08学的概念：
 *
 *   EP07 信息隐藏：
 *     .h = 菜单（顾客能看到的）
 *     .c = 后厨（顾客看不到的）
 *     这个头文件就是"菜单"——只暴露5个函数。
 *     具体怎么操作寄存器？那是.c文件（后厨）的事。
 *
 *   EP08 手搓class：
 *     HAL_GPIO_ 前缀 = 类名。
 *     就像 led_ 前缀代表LED类，motor_ 前缀代表电机类，
 *     HAL_GPIO_ 前缀代表"GPIO类"——看到前缀就知道是谁家的。
 *
 *     HAL_GPIO_Init   = 构造函数（开门营业）
 *     HAL_GPIO_DeInit = 析构函数（打烊收工）
 *
 *   注意每个函数第一个参数：GPIO_TypeDef *GPIOx
 *     这就是EP06的me指针！"我操作的是哪个端口"。
 *
 * 真实HAL库：stm32f4xx_hal_gpio.h
 * 我们的：   hal_gpio.h
 * 接口设计一模一样——因为套路一模一样。
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "gpio_typedef.h"

/* ================================================================
 *  HAL GPIO 公共接口（= 菜单）
 *
 *  五个函数，每个第一个参数都是 GPIO_TypeDef *GPIOx。
 *  GPIOx 就是 me 指针——"我操作的是哪个GPIO端口"。
 *
 *  真实HAL库：
 *    void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *init);
 *  我们的：
 *    void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *init);
 *
 *  签名一模一样。
 * ================================================================ */

/**
 * @brief  初始化GPIO引脚（= 构造函数）
 *
 * 根据init结构体中的配置，设置GPIOx端口的指定引脚。
 * 和 led_init(&led, pin) 是一个套路——
 * 第一个参数是"谁"，后面是"怎么配"。
 *
 * @param  GPIOx  GPIO端口指针（GPIOA / GPIOB / GPIOC）—— me指针
 * @param  init   初始化配置结构体指针
 */
void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *init);

/**
 * @brief  反初始化GPIO引脚（= 析构函数）
 *
 * 将指定引脚恢复为默认状态（输入模式、无上下拉）。
 * 和 led_deinit(&led) 是一个套路。
 *
 * @param  GPIOx  GPIO端口指针 —— me指针
 * @param  pin    要反初始化的引脚
 */
void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t pin);

/**
 * @brief  写引脚电平
 *
 * 和 led_on(&led) / led_off(&led) 是一个套路——
 * 操作的是GPIOx这个"实例"的指定引脚。
 *
 * @param  GPIOx  GPIO端口指针 —— me指针
 * @param  pin    引脚编号
 * @param  value  true=高电平, false=低电平
 */
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t pin, bool value);

/**
 * @brief  读引脚电平
 *
 * @param  GPIOx  GPIO端口指针 —— me指针
 * @param  pin    引脚编号
 * @return true=高电平, false=低电平
 */
bool HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t pin);

/**
 * @brief  翻转引脚电平
 *
 * 和 led_toggle(&led) 是一个套路。
 *
 * @param  GPIOx  GPIO端口指针 —— me指针
 * @param  pin    引脚编号
 */
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t pin);

#endif /* HAL_GPIO_H */
