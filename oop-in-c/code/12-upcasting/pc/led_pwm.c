/* SPDX-License-Identifier: MIT */
/**
 * @file  led_pwm.c
 * @brief PWM 子类 init + 实现层 + led_ops_pwm 操作表 (ch12 版)
 *
 * @details
 * pwm_on / pwm_off 函数签名都是 (struct led_base *me). 第一行
 * (struct led_pwm *)me 强转回子类拿 channel / duty 字段, 合法因为
 * base 在 led_pwm 的第 0 字段.
 *
 * PC 教学版用 printf 模拟一次 PWM 设占空比动作. STM32 上真机会调
 * HAL_TIM_PWM_Start + __HAL_TIM_SET_COMPARE.
 */

#include "led_pwm.h"
#include <stdio.h>

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	printf("  [%s] PWM ch%u duty=%u%%\n",
	       me->name, (unsigned)self->channel, (unsigned)self->duty);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	printf("  [%s] PWM ch%u duty=0%%\n",
	       me->name, (unsigned)self->channel);
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
	me->duty = duty;
	return 0;
}
