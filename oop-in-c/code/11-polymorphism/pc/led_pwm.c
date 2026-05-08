/* SPDX-License-Identifier: MIT */
/**
 * @file  led_pwm.c
 * @brief PWM 子类 init + 实现层 + led_ops_pwm 操作表
 */

#include "led_pwm.h"
#include <stdio.h>

static const struct led_ops led_ops_pwm;

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
	printf("  [PWM] sub-class init done (channel=%u, duty=%u)\n",
	       (unsigned)channel, (unsigned)duty);
	return 0;
}

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	me->is_on = true;
	printf("  [PWM] \"%s\" ON  (channel %u, duty=%u)\n",
	       me->name, (unsigned)self->channel, (unsigned)self->duty);
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	me->is_on = false;
	printf("  [PWM] \"%s\" OFF (channel %u)\n",
	       me->name, (unsigned)self->channel);
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
