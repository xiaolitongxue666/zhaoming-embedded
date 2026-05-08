/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.c - 子类二: PWM LED 实现 (ch15 完整版, 风格 A)
 *
 * 这一份只负责 PWM 子类: 三件套全填 (pwm_on / pwm_off / pwm_set_brightness)
 * + ops 表 (pwm_ops) + led_pwm_init 构造函数.
 *
 * 子类只调 platform_pwm_xxx ops 表层接口 (platform_pwm_enable /
 * platform_pwm_disable / platform_pwm_set_duty), 永远不直接碰 PWM 寄存器.
 * platform_pwm dispatcher 启动期由 platform_init 调 platform_pc_pwm_init
 * 注册 PC 后端, STM32 端在 platform/arch/stm32/pin_board.c 里注册 HAL 后端.
 * 同一份 led_pwm.c 在 PC、STM32、NXP 上字节级不动. 见 ch15 § 15.11.5.
 */

#include "led_pwm.h"
#include "container_of.h"
#include "platform/platform_pwm.h"
#include <stdio.h>

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	int rc;

	printf("  [%s] led_on  -> PWM ch%u duty=%u%%\n",
	       me->name, (unsigned)self->channel, (unsigned)self->duty);
	rc = platform_pwm_enable(self->channel);
	if (rc != 0)
		return rc;
	/* duty 是百分比 0-100, platform_pwm_set_duty 接 0-255, 线性映射. */
	rc = platform_pwm_set_duty(self->channel,
	                           (uint8_t)((uint32_t)self->duty * 255U / 100U));
	if (rc != 0)
		return rc;
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);

	printf("  [%s] led_off -> PWM ch%u duty=0%%\n",
	       me->name, (unsigned)self->channel);
	(void)platform_pwm_set_duty(self->channel, 0);
	(void)platform_pwm_disable(self->channel);
	me->is_on = false;
	return 0;
}

static int pwm_set_brightness(struct led_base *me, uint8_t brightness)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);

	if (brightness > 100)
		brightness = 100;
	self->duty = brightness;
	printf("  [%s] set_brightness -> PWM ch%u duty=%u%%\n",
	       me->name, (unsigned)self->channel, (unsigned)brightness);
	if (me->is_on)
		(void)platform_pwm_set_duty(self->channel,
		                            (uint8_t)((uint32_t)brightness * 255U / 100U));
	me->is_on = (brightness > 0);
	return 0;
}

static const struct led_ops pwm_ops = {
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
	rc = led_base_init(&me->base, name, &pwm_ops);
	if (rc != 0)
		return rc;
	me->channel = channel;
	me->duty    = duty;
	return 0;
}
