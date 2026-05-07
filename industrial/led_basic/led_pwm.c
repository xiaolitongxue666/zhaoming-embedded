/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.c - LED PWM 子类实现.
 *
 * 上电默认 brightness = 255 (全亮). 后续 led_set_brightness 改 brightness,
 * 下次 led_on 用新值. led_off 把 duty 写 0.
 */

#include <stddef.h>

#include "led_pwm.h"
#include "platform_pwm.h"

#define LED_PWM_DEFAULT_BRIGHTNESS    255

static platform_err_t _led_pwm_on(struct led_base *me);
static platform_err_t _led_pwm_off(struct led_base *me);
static platform_err_t _led_pwm_set_brightness(
	struct led_base *me, uint8_t level);

static const struct led_ops led_pwm_ops = {
	.on             = _led_pwm_on,
	.off            = _led_pwm_off,
	.set_brightness = _led_pwm_set_brightness,
};

platform_err_t led_pwm_init(struct led_pwm *me, const char *name,
                            int32_t channel)
{
	platform_err_t ret;

	if (NULL == me) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	me->channel    = channel;
	me->brightness = LED_PWM_DEFAULT_BRIGHTNESS;

	(void)platform_pwm_disable(channel);
	(void)platform_pwm_set_duty(channel, 0);

	ret = led_base_init(&me->base, name, &led_pwm_ops);

exit:
	return ret;
}

static platform_err_t _led_pwm_on(struct led_base *me)
{
	struct led_pwm *pwm = (struct led_pwm *)me;
	platform_err_t  ret;

	ret = platform_pwm_enable(pwm->channel);
	if (PLATFORM_EOK != ret) {
		goto exit;
	}

	ret = platform_pwm_set_duty(pwm->channel, pwm->brightness);

exit:
	return ret;
}

static platform_err_t _led_pwm_off(struct led_base *me)
{
	struct led_pwm *pwm = (struct led_pwm *)me;

	(void)platform_pwm_set_duty(pwm->channel, 0);
	return platform_pwm_disable(pwm->channel);
}

static platform_err_t _led_pwm_set_brightness(
	struct led_base *me, uint8_t level)
{
	struct led_pwm *pwm = (struct led_pwm *)me;
	platform_err_t  ret = PLATFORM_EOK;

	pwm->brightness = level;

	/* 如果当前是亮的, 立刻把新亮度推下去, 否则只更新缓存值 */
	if (me->is_on) {
		ret = platform_pwm_set_duty(pwm->channel, level);
	}

	return ret;
}
