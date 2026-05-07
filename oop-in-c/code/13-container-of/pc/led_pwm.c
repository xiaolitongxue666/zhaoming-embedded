/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.c - PWM 子类 init + 实现层 + led_ops_pwm 操作表 (ch13 版)
 *
 * 三个 static 函数 pwm_on / pwm_off / pwm_set_brightness 都先用
 * container_of 把 base 指针反推回 struct led_pwm * 拿 channel / duty
 * 字段. PWM 子类 base 在第 0 字段, container_of 在这种布局下偏移
 * offsetof = 0, 减法就是减 0, 编译器会优化掉, 等价于直接强转.
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

const struct led_ops led_ops_pwm = {
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
