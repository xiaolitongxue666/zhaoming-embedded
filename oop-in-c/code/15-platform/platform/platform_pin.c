/* SPDX-License-Identifier: MIT */
/*
 * platform_pin.c - PIN 抽象层 dispatcher.
 *
 * 维护 static const struct platform_pin_ops *_g_ops, 上层调
 * platform_pin_xxx 时内部 dispatch. 子类启动期调
 * platform_pin_register(&_xxx_pin_ops) 把指针填进来.
 *
 * 这一份和 platform/arch/<mcu>/pin_board.c 是 ch15 platform 抽象层的两个
 * 角色: 接口层 (本文件) 跨 MCU 不变, 实现层 (pin_board.c) 各 MCU 各一份.
 */

#include "platform/platform_pin.h"
#include <assert.h>
#include <stddef.h>

static const struct platform_pin_ops *_g_ops = NULL;

int platform_pin_register(const struct platform_pin_ops *ops)
{
	if (!ops)
		return -1;
	_g_ops = ops;
	return 0;
}

void platform_pin_mode(int32_t pin, int32_t mode)
{
	assert(_g_ops && _g_ops->mode);
	_g_ops->mode(pin, mode);
}

void platform_pin_write(int32_t pin, int32_t value)
{
	assert(_g_ops && _g_ops->write);
	_g_ops->write(pin, value);
}

int32_t platform_pin_read(int32_t pin)
{
	assert(_g_ops && _g_ops->read);
	return _g_ops->read(pin);
}

int32_t platform_pin_get(const char *name)
{
	assert(_g_ops);
	if (!_g_ops->get)
		return -1;
	return _g_ops->get(name);
}
