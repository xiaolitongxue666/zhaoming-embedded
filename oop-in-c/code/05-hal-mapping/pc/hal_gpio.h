/* SPDX-License-Identifier: MIT */
/*
 * hal_gpio.h - 模拟 HAL 库 GPIO 公共接口（菜单）
 *
 * 见书 ch05 § 5.7 完整映射表。
 *
 * 这一章是验证课。这个 .h 里的接口和真实 stm32h7xx_hal_gpio.h 几乎
 * 一字不差。你能在这里看到 ch01 到 ch04 的所有概念落地到工业 HAL 库：
 *
 *   .h 暴露公开接口（ch02 菜单）
 *   .c 把字段定义和 static 工具函数关起来（ch02 后厨）
 *   HAL_GPIO_ 前缀 = 类名（ch03 命名规范）
 *   HAL_GPIO_Init / HAL_GPIO_DeInit = 构造 / 析构（ch03 init/deinit）
 *   GPIO_TypeDef *GPIOx 这个第一参数 = me 指针（ch01）
 *   GPIOA / GPIOB / GPIOC = 同一个 struct 的多实例（ch01）
 *   配置参数装进 GPIO_InitTypeDef = 把"开门营业的初始状态"打包成 struct
 *
 * **几千个 HAL 函数就一个套路**: HAL_UART_Init / HAL_SPI_Init /
 * HAL_I2C_Init / HAL_TIM_Base_Init / HAL_ADC_Init... 每个外设都是
 * <MOD>_TypeDef + <MOD>_Init / DeInit + 操作函数 这套, 每个操作
 * 函数底下都是寄存器 store。
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "gpio_typedef.h"

/*
 * HAL_GPIO_Init -- 构造函数
 * 真实签名（stm32h7xx_hal_gpio.h）：
 *   void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *Init);
 */
void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *init);

/* HAL_GPIO_DeInit -- 析构函数 */
void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t pin);

/* HAL_GPIO_WritePin -- 写引脚电平 */
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t pin, bool value);

/* HAL_GPIO_ReadPin -- 读引脚电平 */
bool HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t pin);

/* HAL_GPIO_TogglePin -- 翻转引脚 */
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t pin);

#endif /* HAL_GPIO_H */
