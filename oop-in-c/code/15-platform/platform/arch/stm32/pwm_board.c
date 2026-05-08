/* SPDX-License-Identifier: MIT */
/*
 * platform/arch/stm32/pwm_board.c - STM32F4 端 PWM 后端实现.
 *
 * "认识 STM32 TIM 的 PWM 寄存器"的唯一一份文件. 上层 drivers/led/led_pwm
 * 调 platform_pwm_xxx ops 表层接口下来, 落到这里之后才碰 TIM_HandleTypeDef *
 * 加 HAL_TIM_PWM_Start / __HAL_TIM_SET_COMPARE.
 *
 * 跨 MCU (STM32 -> NXP) 的"换 6 份之一": 同目录 pin_board.c (GPIO) + 本文
 * (TIM PWM) + i2c_board.c (I2C bus) 三份, 是这家 MCU 的 platform 实现层.
 * 上层 drivers/led/led_pwm + platform/platform_pwm.* 字节级不动.
 *
 * 工程假设: STM32CubeMX 已配好 TIM3 (4 路 PWM, ARR=999, 1 kHz), 生成
 * htim3 全局句柄. 真实工程改 #include 头 + 改通道映射即可.
 */

#include "platform/platform_pwm.h"
#include "stm32f4xx_hal.h"

/* CubeMX 生成的外设句柄. 这里仅声明, 真机工程在 main.c / *_msp.c 给出定义. */
extern TIM_HandleTypeDef htim3;

static uint32_t _pwm_channel_to_hal(int32_t channel)
{
	switch (channel) {
	case 1:  return TIM_CHANNEL_1;
	case 2:  return TIM_CHANNEL_2;
	case 3:  return TIM_CHANNEL_3;
	case 4:  return TIM_CHANNEL_4;
	default: return TIM_CHANNEL_1;
	}
}

static int _stm32_pwm_enable(int32_t channel)
{
	HAL_TIM_PWM_Start(&htim3, _pwm_channel_to_hal(channel));
	return 0;
}

static int _stm32_pwm_disable(int32_t channel)
{
	HAL_TIM_PWM_Stop(&htim3, _pwm_channel_to_hal(channel));
	return 0;
}

static int _stm32_pwm_set_duty(int32_t channel, uint8_t duty)
{
	/* ARR=999, 8-bit duty 线性映射到 0-1000 */
	uint32_t ccr = (uint32_t)duty * 1000U / 255U;
	__HAL_TIM_SET_COMPARE(&htim3, _pwm_channel_to_hal(channel), ccr);
	return 0;
}

static const struct platform_pwm_ops _stm32_pwm_ops = {
	.enable   = _stm32_pwm_enable,
	.disable  = _stm32_pwm_disable,
	.set_duty = _stm32_pwm_set_duty,
};

/* 启动期由 platform_init 调一次. 调用顺序: pin -> pwm -> i2c. */
void platform_hw_pwm_init(void)
{
	(void)platform_pwm_register(&_stm32_pwm_ops);
}
