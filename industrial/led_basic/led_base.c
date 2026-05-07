/* SPDX-License-Identifier: MIT */
/*
 * led_base.c - LED 基类 dispatch + 默认实现.
 *
 * 父类层在调用子类 ops 之前 assert 收拢必填项校验:
 *   1. me 本身合法
 *   2. me->ops 已填充
 *   3. 子类必填的 ops 函数指针 已填充
 *
 * 三层校验防示子类忘填纯虚函数, 或实例初始化路径出错使 ops 仅部分填充.
 */

#include <stddef.h>

#include "led_base.h"
#include "platform_assert.h"

platform_err_t led_base_init(struct led_base *me, const char *name,
                             const struct led_ops *ops)
{
	platform_err_t ret = PLATFORM_EOK;

	if ((NULL == me) || (NULL == name) || (NULL == ops)) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	me->ops   = ops;
	me->name  = name;
	me->is_on = false;

exit:
	return ret;
}

platform_err_t led_on(struct led_base *me)
{
	platform_err_t ret;

	if (NULL == me) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	platform_assert(me->ops != NULL);
	platform_assert(me->ops->on != NULL);   /* 纯虚必填 */

	ret = me->ops->on(me);
	if (PLATFORM_EOK == ret) {
		me->is_on = true;
	}

exit:
	return ret;
}

platform_err_t led_off(struct led_base *me)
{
	platform_err_t ret;

	if (NULL == me) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	platform_assert(me->ops != NULL);
	platform_assert(me->ops->off != NULL);  /* 纯虚必填 */

	ret = me->ops->off(me);
	if (PLATFORM_EOK == ret) {
		me->is_on = false;
	}

exit:
	return ret;
}

platform_err_t led_set_brightness(struct led_base *me, uint8_t level)
{
	platform_err_t ret;

	if (NULL == me) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	platform_assert(me->ops != NULL);

	if (NULL == me->ops->set_brightness) {
		/* 选填: 父类默认 no-op (GPIO LED / I2C 简单 LED 不支持调亮度) */
		ret = PLATFORM_EOK;
		goto exit;
	}

	ret = me->ops->set_brightness(me, level);

exit:
	return ret;
}
