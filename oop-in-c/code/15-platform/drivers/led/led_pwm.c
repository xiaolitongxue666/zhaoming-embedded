/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.c - LED PWM 子类实现.
 *
 * 上电默认 brightness = 255 (全亮). 后续 led_set_brightness 改 brightness,
 * 下次 led_on 用新值. led_off 把 duty 写 0.
 *
 * 子类只调 platform_pwm_xxx 封装函数, 跨 MCU 0 改动. 见 ch15 § 15.11.5.
 */

#include "drivers/led/led_pwm.h"
#include "platform/platform_pwm.h"
#include <stddef.h>

#define LED_PWM_DEFAULT_BRIGHTNESS    255

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	int rc;

	rc = platform_pwm_enable(self->channel);
	if (rc != 0)
		return rc;
	return platform_pwm_set_duty(self->channel, self->brightness);
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;

	(void)platform_pwm_set_duty(self->channel, 0);
	return platform_pwm_disable(self->channel);
}

static int pwm_set_brightness(struct led_base *me, uint8_t level)
{
	struct led_pwm *self = (struct led_pwm *)me;
	int rc = 0;

	self->brightness = level;
	if (me->is_on)
		rc = platform_pwm_set_duty(self->channel, level);
	return rc;
}

static const struct led_ops pwm_ops = {
	.on             = pwm_on,
	.off            = pwm_off,
	.set_brightness = pwm_set_brightness,
};

int led_pwm_init(struct led_pwm *me, const char *name, int32_t channel)
{
	if (!me)
		return -1;

	me->channel    = channel;
	me->brightness = LED_PWM_DEFAULT_BRIGHTNESS;

	(void)platform_pwm_disable(channel);
	(void)platform_pwm_set_duty(channel, 0);

	return led_base_init(&me->base, name, &pwm_ops);
}
