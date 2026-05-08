/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.c - PWM 子类 init + 实现层 + led_ops_pwm 操作表 (ch14 版)
 *
 * PWM 子类三件套全填. 跟 GPIO 子类合在一起演示 ch14 的"选填"策略:
 * GPIO 没填 set_brightness 走父类默认, PWM 填了走 pwm_set_brightness.
 *
 * 子类实现 (pwm_on / pwm_off / pwm_set_brightness) 接 struct led_base *,
 * 第一行用 container_of 反推回子类指针, 跟 ch13 风格一致.
 */

#include "led_pwm.h"
#include "container_of.h"
#include <stdio.h>

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	printf("  [%s] PWM ch%u duty=%u%%\n",
	       me->name, (unsigned)self->channel, (unsigned)self->duty);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	printf("  [%s] PWM ch%u duty=0%%\n",
	       me->name, (unsigned)self->channel);
	me->is_on = false;
	return 0;
}

static int pwm_set_brightness(struct led_base *me, uint8_t brightness)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	if (brightness > 100)
		brightness = 100;
	self->duty = brightness;
	printf("  [%s] PWM ch%u duty=%u%%\n",
	       me->name, (unsigned)self->channel, (unsigned)brightness);
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
