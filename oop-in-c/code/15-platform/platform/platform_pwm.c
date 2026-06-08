/* SPDX-License-Identifier: MIT */
/*
 * platform_pwm.c - PWM dispatcher (分发器).
 *
 * 设计: 存一张 ops 指针 _g_ops; 启动期 platform_pwm_register() 挂后端;
 * 运行时 platform_pwm_enable() 等固定 API 内部转发到 _g_ops->enable 等.
 * 与 ch11 led_base + me->ops 同构, ops 对象换成 MCU 后端.
 *
 * 调用链 (PC):
 *   led_pwm.c: platform_pwm_enable(ch)
 *     → 本文件: _g_ops->enable(ch)
 *       → platform_pwm_pc.c: _pc_pwm_enable(ch) → printf
 *
 * 真机后端: platform/arch/<mcu>/pwm_board.c 调 __HAL_TIM_SET_COMPARE 等.
 * 上层 led_pwm.c 字节不动. 详见 15-platform/pc/README.md.
 *
 * 与 platform_pin / platform_i2c 同款: static _g_ops, register 填表,
 * 公共 API dispatch. 启动期由 platform_init (PC) 或 platform_hw_pwm_init
 * (真机) 触发 register.
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
