/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.c
 * @brief 基类层实现 - 只填 vptr 字段, 真正 dispatch 在 led.c
 *
 * @details
 * 这一层是 led 子系统对 platform 层的边界:
 *   - 不直接碰 platform_ops 表
 *   - 也不知道 platform 当前选的是 PC / STM32 / Linux 哪个
 *   - 任何 platform 操作都通过 platform_gpio_xxx 封装函数, 签名
 *     从 ch01 起一字不变
 *
 * platform 层内部从 ch11 起换成 ops 表 dispatch (见 platform_ops.h),
 * 但封装函数对外形态没变, 因此 led_base.c 一字不动.
 *
 * 这就是工业代码"对外稳定、对内可换"的分层: 上层 (led) 不知道
 * 下层 (platform) 内部用了什么实现策略, 也不关心.
 */

#include "led_base.h"
#include "led.h"
#include <stdio.h>

int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops)
{
	if (!me || !name || !ops)
		return -1;

	me->ops = ops;
	me->name = name;
	me->is_on = false;

	printf("  [base] \"%s\" common init done, ops=%p\n",
	       name, (const void *)ops);
	return 0;
}

const char *led_base_get_name(const struct led_base *me)
{
	if (!me)
		return "(null)";
	return me->name;
}
