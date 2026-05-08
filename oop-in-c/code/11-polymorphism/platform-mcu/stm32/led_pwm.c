/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.c - LED PWM 子类 STM32 端真机实现
 *
 * 这是子类内部的 STM32 真机版本 (片段, 不是完整工程). 完整 STM32 工程
 * 见附录 B (Zephyr v3.7.0 LTS · stm32f4_disco).
 *
 * 父类 led_on / led_off / led_toggle 写在 led_base.c, 子类实现 (pwm_on)
 * 直接调 __HAL_TIM_SET_COMPARE + HAL_TIM_PWM_Start / Stop. 跟 pc/ 唯一
 * 的差别就在这个文件: 把 printf 模拟换成真实 HAL 操作. 应用层 / 父类 /
 * ops 表 / 子类 init 一字不改.
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

static int pwm_toggle(struct led_base *me)
{
	if (me->is_on)
		return pwm_off(me);
	return pwm_on(me);
}

static const struct led_ops led_ops_pwm = {
	.on     = pwm_on,
	.off    = pwm_off,
	.toggle = pwm_toggle,
};
