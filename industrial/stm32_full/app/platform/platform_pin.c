/* SPDX-License-Identifier: MIT */
/*
 * platform_pin.c - PIN framework dispatcher.
 *
 * 维护 static const struct platform_pin_ops *_g_ops, 上层调 platform_pin_xxx
 * 时内部 dispatch. 子类启动期调 platform_pin_register(&_xxx_ops) 把指针填进来.
 */

#include <stddef.h>

#include "platform/platform_pin.h"
#include "platform/platform_assert.h"

static const struct platform_pin_ops *_g_ops = NULL;

platform_err_t platform_pin_register(const struct platform_pin_ops *ops)
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

void platform_pin_mode(int32_t pin, int32_t mode)
{
	platform_assert(_g_ops != NULL);
	platform_assert(_g_ops->mode != NULL);
	_g_ops->mode(pin, mode);
}

void platform_pin_write(int32_t pin, int32_t value)
{
	platform_assert(_g_ops != NULL);
	platform_assert(_g_ops->write != NULL);
	_g_ops->write(pin, value);
}

int32_t platform_pin_read(int32_t pin)
{
	platform_assert(_g_ops != NULL);
	platform_assert(_g_ops->read != NULL);
	return _g_ops->read(pin);
}

platform_err_t platform_pin_attach_irq(
	int32_t pin, uint32_t mode, void (*hdr)(void *args), void *args)
{
	platform_err_t ret = PLATFORM_ERROR;

	platform_assert(_g_ops != NULL);
	if (NULL != _g_ops->attach_irq) {
		ret = _g_ops->attach_irq(pin, mode, hdr, args);
		goto exit;
	}
	ret = PLATFORM_ENOSYS;

exit:
	return ret;
}

platform_err_t platform_pin_detach_irq(int32_t pin)
{
	platform_err_t ret = PLATFORM_ERROR;

	platform_assert(_g_ops != NULL);
	if (NULL != _g_ops->detach_irq) {
		ret = _g_ops->detach_irq(pin);
		goto exit;
	}
	ret = PLATFORM_ENOSYS;

exit:
	return ret;
}

platform_err_t platform_pin_irq_enable(int32_t pin, uint32_t enabled)
{
	platform_err_t ret = PLATFORM_ERROR;

	platform_assert(_g_ops != NULL);
	if (NULL != _g_ops->irq_enable) {
		ret = _g_ops->irq_enable(pin, enabled);
		goto exit;
	}
	ret = PLATFORM_ENOSYS;

exit:
	return ret;
}

int32_t platform_pin_get(const char *name)
{
	int32_t ret = -1;

	platform_assert(_g_ops != NULL);
	platform_assert(name != NULL);
	platform_assert(name[0] == 'P');

	if (NULL == _g_ops->get) {
		ret = PLATFORM_ENOSYS;
		goto exit;
	}

	ret = _g_ops->get(name);

exit:
	return ret;
}
