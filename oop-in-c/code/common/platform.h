/**
 * @file    platform.h
 * @brief   平台抽象层 - GPIO操作接口
 * @author  兆鸣嵌入式
 * @note    这就是视频里教的"平台抽象"——同一套接口，不同的实现。
 *          PC上用printf模拟，STM32上操作真实硬��。
 *          上层代码一行不改。
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

/* GPIO模式定义 */
#define GPIO_MODE_OUTPUT    0   /* 输出模式 */
#define GPIO_MODE_INPUT     1   /* 输入模式 */

/**
 * @brief  初始化一个GPIO引脚
 * @param  pin   引脚号
 * @param  mode  GPIO_MODE_OUTPUT 或 GPIO_MODE_INPUT
 */
void platform_gpio_init(uint8_t pin, uint8_t mode);

/**
 * @brief  反初始化一个GPIO引脚（释放资源）
 * @param  pin  引脚号
 */
void platform_gpio_deinit(uint8_t pin);

/**
 * @brief  写GPIO电平
 * @param  pin    引脚号
 * @param  value  true=高电平, false=低电平
 */
void platform_gpio_write(uint8_t pin, bool value);

/**
 * @brief  读GPIO电平
 * @param  pin  引脚号
 * @return true=高电平, false=低电平
 */
bool platform_gpio_read(uint8_t pin);

#endif /* PLATFORM_H */
