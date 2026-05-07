/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.c - LED PWM 子类 STM32 端真机实现 (ch12 版)
 *
 * 这是子类内部的 STM32 真机版本 (片段, 不是完整工程). 完整 STM32 工程
 * 见附录 B (industrial/stm32_full).
 *
 * pwm_on / pwm_off 函数体里 (struct led_pwm *)me 强转回子类拿 channel /
 * duty 字段, 走真实 TIM PWM 寄存器. __HAL_TIM_SET_COMPARE 改 CCR 寄存器,
 * HAL_TIM_PWM_Start / Stop 控通道使能位. duty 字段 0-100 (百分比) 转
 * CCR 值, ARR 假设 1000 (CubeMX 配 TIM3 ARR=999 + 1kHz 频率 时这一档
 * 刚好).
 *
 * 真实工程里 PWM 句柄通过 CubeMX 生成的全局变量拿到 (htim3), 这里片段
 * 用 extern 声明引用.
 */

#include "led_pwm.h"
#include "stm32f4xx_hal.h"

/* CubeMX 生成的外设句柄 (PWM 走 TIM3). */
extern TIM_HandleTypeDef htim3;

static uint32_t pwm_channel_to_hal(uint8_t channel)
{
	switch (channel) {
	case 1:  return TIM_CHANNEL_1;
	case 2:  return TIM_CHANNEL_2;
	case 3:  return TIM_CHANNEL_3;
	case 4:  return TIM_CHANNEL_4;
	default: return TIM_CHANNEL_1;
	}
}

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	uint32_t ch  = pwm_channel_to_hal(self->channel);
	uint32_t ccr = (uint32_t)self->duty * 1000U / 100U;

	__HAL_TIM_SET_COMPARE(&htim3, ch, ccr);
	HAL_TIM_PWM_Start(&htim3, ch);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	uint32_t ch = pwm_channel_to_hal(self->channel);

	__HAL_TIM_SET_COMPARE(&htim3, ch, 0);
	HAL_TIM_PWM_Stop(&htim3, ch);
	me->is_on = false;
	return 0;
}

const struct led_ops led_ops_pwm = {
	.on  = pwm_on,
	.off = pwm_off,
};

int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty)
{
	int rc;
	if (!me)
		return -1;
	if (duty > 100)
		return -2;
	rc = led_base_init(&me->base, name, &led_ops_pwm);
	if (rc != 0)
		return rc;
	me->channel = channel;
	me->duty    = duty;
	return 0;
}
