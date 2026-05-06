/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.c
 * @brief 基类实现 + 回调注册接口
 */
#include "led_base.h"
#include <stdio.h>

int led_base_init(struct led_base *me, const char *name)
{
	if (!me || !name)
		return -1;
	me->name = name;
	me->is_on = false;
	/*
	 * 回调字段默认 NULL. 没人注册时 led_on/off 不会调它.
	 * 不能让字段保持未初始化状态: 栈上分配的 led_base 如果不显式置 NULL,
	 * 下面 led_on 里的 if (me->on_state_change) 可能读到野指针, 跳飞.
	 */
	me->on_state_change = NULL;
	printf("  [base] \"%s\" common init done\n", name);
	return 0;
}

/*
 * 注册回调 - 应用层把自己的函数地址塞进来.
 *
 * 传 NULL 等价于"撤销注册" (见 ch08 § 8.6.7). 工业代码里也常见
 * 单独的 led_unregister_state_cb 接口, 本章用合并版.
 *
 * 真实嵌入式工程里这个调用绝大多数发生在启动期 (一次配置后不再动),
 * 因此本实现没做并发保护. 如果要支持运行期注册/撤销, 见 ch08 § 8.6.2.2
 * 的三种处理: 永不撤销 / atomic / 关中断包围.
 */
int led_register_state_cb(struct led_base *me, led_state_cb cb)
{
	if (!me)
		return -1;
	me->on_state_change = cb;
	if (cb)
		printf("  [base] \"%s\" state callback registered\n", me->name);
	else
		printf("  [base] \"%s\" state callback cleared\n", me->name);
	return 0;
}

const char *led_base_get_name(const struct led_base *me)
{
	if (!me)
		return "(null)";
	return me->name;
}
