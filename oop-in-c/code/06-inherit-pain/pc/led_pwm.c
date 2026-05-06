/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.c - LED PWM 子类的实现
 *
 * led_pwm_init 第一行也调 led_base_init, 和 led_gpio_init 套路一致.
 * 然后处理 PWM 自己的字段 (channel, duty).
 */

#include "led_pwm.h"
#include <stdio.h>

int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty)
{
	int rc;

	if (!me)
		return -1;
	if (duty > 100)
		return -2;

	rc = led_base_init(&me->base, name);
	if (rc != 0)
		return rc;

	me->channel = channel;
	me->duty = duty;

	printf("  [PWM] sub-class init done (channel=%u, duty=%u%%)\n",
	       (unsigned)channel, (unsigned)duty);
	return 0;
}

int led_pwm_on(struct led_pwm *me)
{
	if (!me)
		return -1;

	me->base.is_on = true;
	printf("  [PWM] \"%s\" ON (channel %u, duty=%u%%)\n",
	       me->base.name, (unsigned)me->channel, (unsigned)me->duty);
	return 0;
}

int led_pwm_off(struct led_pwm *me)
{
	if (!me)
		return -1;

	me->base.is_on = false;
	printf("  [PWM] \"%s\" OFF (channel %u)\n",
	       me->base.name, (unsigned)me->channel);
	return 0;
}
