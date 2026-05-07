/* SPDX-License-Identifier: MIT */
/*
 * platform_pwm.c - PWM 抽象层 dispatcher.
 *
 * 与 platform_pin 同款结构: static const struct platform_pwm_ops *_g_ops,
 * 启动期由 pin_board.c (合并 PWM 实现) 调 platform_pwm_register 填进来.
 */

#include "platform/platform_pwm.h"
#include <assert.h>
#include <stddef.h>

static const struct platform_pwm_ops *_g_ops = NULL;

int platform_pwm_register(const struct platform_pwm_ops *ops)
{
	if (!ops)
		return -1;
	_g_ops = ops;
	return 0;
}

int platform_pwm_enable(int32_t channel)
{
	assert(_g_ops);
	if (!_g_ops->enable)
		return -1;
	return _g_ops->enable(channel);
}

int platform_pwm_disable(int32_t channel)
{
	assert(_g_ops);
	if (!_g_ops->disable)
		return -1;
	return _g_ops->disable(channel);
}

int platform_pwm_set_duty(int32_t channel, uint8_t duty)
{
	assert(_g_ops);
	if (!_g_ops->set_duty)
		return -1;
	return _g_ops->set_duty(channel, duty);
}
