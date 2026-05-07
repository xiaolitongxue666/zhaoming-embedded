/* SPDX-License-Identifier: MIT */
/*
 * platform_pwm.c - PWM framework dispatcher.
 *
 * 与 platform_pin 同款结构: static const struct platform_pwm_ops *_g_ops,
 * 启动期由 pwm_board.c 调 platform_pwm_register 填进来.
 */

#include <stddef.h>

#include "platform/platform_pwm.h"
#include "platform/platform_assert.h"

static const struct platform_pwm_ops *_g_ops = NULL;

platform_err_t platform_pwm_register(const struct platform_pwm_ops *ops)
{
	platform_err_t ret = PLATFORM_EINVAL;

	if (NULL == ops) {
		goto exit;
	}

	_g_ops = ops;
	ret = PLATFORM_EOK;

exit:
	return ret;
}

platform_err_t platform_pwm_enable(int32_t channel)
{
	platform_err_t ret = PLATFORM_ENOSYS;

	platform_assert(_g_ops != NULL);
	if (NULL != _g_ops->enable) {
		ret = _g_ops->enable(channel);
	}

	return ret;
}

platform_err_t platform_pwm_disable(int32_t channel)
{
	platform_err_t ret = PLATFORM_ENOSYS;

	platform_assert(_g_ops != NULL);
	if (NULL != _g_ops->disable) {
		ret = _g_ops->disable(channel);
	}

	return ret;
}

platform_err_t platform_pwm_set_duty(int32_t channel, uint8_t duty)
{
	platform_err_t ret = PLATFORM_ENOSYS;

	platform_assert(_g_ops != NULL);
	if (NULL != _g_ops->set_duty) {
		ret = _g_ops->set_duty(channel, duty);
	}

	return ret;
}
