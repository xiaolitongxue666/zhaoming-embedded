/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.c - LED PWM 子类 STM32 端真机实现 (ch13 版)
 *
 * 这是子类内部的 STM32 真机版本 (片段, 不是完整工程). 完整 STM32 工程
 * 见附录 B (Zephyr v3.7.0 LTS · stm32f4_disco).
 *
 * pwm_xxx 函数体里第一行用 container_of 反推自己, 后面调真实 TIM PWM
 * 寄存器. duty 字段 0-100 (百分比) 转 CCR 值, ARR 假设 1000.
 *
 * 跟 pc/ 唯一的差别: 把 printf 模拟换成真实 HAL 操作.
 *
 * 真实工程里 PWM 句柄通过 CubeMX 生成的全局变量拿到 (htim3), 这里片段
 * 用 extern 声明引用.
 */

#include "led_pwm.h"
#include "container_of.h"
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
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	uint32_t ch  = pwm_channel_to_hal(self->channel);
	uint32_t ccr = (uint32_t)self->duty * 1000U / 100U;

	__HAL_TIM_SET_COMPARE(&htim3, ch, ccr);
	HAL_TIM_PWM_Start(&htim3, ch);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	uint32_t ch = pwm_channel_to_hal(self->channel);

	__HAL_TIM_SET_COMPARE(&htim3, ch, 0);
	HAL_TIM_PWM_Stop(&htim3, ch);
	me->is_on = false;
	return 0;
}

static int pwm_set_brightness(struct led_base *me, uint8_t brightness)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	uint32_t ch  = pwm_channel_to_hal(self->channel);
	uint32_t ccr;

	if (brightness > 100)
		brightness = 100;
	self->duty = brightness;
	ccr = (uint32_t)brightness * 1000U / 100U;

	__HAL_TIM_SET_COMPARE(&htim3, ch, ccr);
	if (brightness > 0)
		HAL_TIM_PWM_Start(&htim3, ch);
	else
		HAL_TIM_PWM_Stop(&htim3, ch);
	me->is_on = (brightness > 0);
	return 0;
}

static const struct led_ops led_ops_pwm = {
	.on             = pwm_on,
	.off            = pwm_off,
	.set_brightness = pwm_set_brightness,
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
