/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - 函数指针当参数 在 STM32 上的样子
 *
 * 三种 on/off 实现走真实硬件: GPIO 拉电平 / PWM 改占空比 / I2C 发命令.
 * 三个函数签名都是 void name(int param), 这样同一个 test_led 用同一对
 * 函数指针参数 void (*)(int) 能匹配任何一组实现.
 *
 * 第三个参数在不同实现里有不同含义: GPIO 是引脚号, PWM 是通道号, I2C
 * 是从机地址. 由调用方负责传匹配的 id.
 *
 * 真实工程里 PWM 通道、I2C 句柄通常通过文件级 static 全局拿到 (CubeMX
 * 生成代码就是这套), 这里片段保留这个风格.
 */

#include "led.h"
#include "stm32f4xx_hal.h"

extern TIM_HandleTypeDef htim3;     /* CubeMX 生成: 控 LED 的 PWM 定时器 */
extern I2C_HandleTypeDef hi2c1;     /* CubeMX 生成: 控 LED 的 I2C 总线 */

void gpio_on(int pin)
{
	HAL_GPIO_WritePin(GPIOA, (uint16_t)(1U << pin), GPIO_PIN_SET);
}

void gpio_off(int pin)
{
	HAL_GPIO_WritePin(GPIOA, (uint16_t)(1U << pin), GPIO_PIN_RESET);
}

void pwm_on(int channel)
{
	__HAL_TIM_SET_COMPARE(&htim3, (uint32_t)channel, 1000);   /* 占空比 100% */
	HAL_TIM_PWM_Start(&htim3, (uint32_t)channel);
}

void pwm_off(int channel)
{
	__HAL_TIM_SET_COMPARE(&htim3, (uint32_t)channel, 0);
	HAL_TIM_PWM_Stop(&htim3, (uint32_t)channel);
}

void i2c_on(int addr)
{
	uint8_t cmd[1] = { 0x01 };       /* 厂家协议: 0x01 = ON */
	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(addr << 1),
	                        cmd, sizeof(cmd), 100);
}

void i2c_off(int addr)
{
	uint8_t cmd[1] = { 0x00 };       /* 厂家协议: 0x00 = OFF */
	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(addr << 1),
	                        cmd, sizeof(cmd), 100);
}
