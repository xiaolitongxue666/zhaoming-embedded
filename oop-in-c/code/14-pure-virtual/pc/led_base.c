/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.c
 * @brief 父类层 -- 共有 init + 父类统一接口 (ch14 版, 三种 NULL 处理策略)
 *
 * @details
 * 父类做两件事:
 *   1) led_base_init - 共有字段 init (ops / name / is_on), 子类 init
 *      第一行调一次. 跟 ch10 / ch11 / ch13 一字不动.
 *   2) led_on / led_off / led_set_brightness - 父类统一接口. ch14
 *      在这里集中演示三种 NULL 处理策略中的两种 (必填 + 选填).
 *
 * "谁来检查 NULL"是这一章的核心抉择. 不在 ops 表本身做手脚 (ops 字段
 * 类型不变, 还是普通函数指针), 把"NULL 不允许"的纪律放到父类统一接口里:
 *
 *   led_on / led_off    -> 必填. ops 没填或为 NULL, assert 报错并返回错误码.
 *                         对应 C++ 纯虚函数 (virtual void f() = 0;).
 *   led_set_brightness  -> 选填. ops 没填, 父类的统一接口走默认行为返回 0.
 *                         对应 C++ 带默认实现的虚函数.
 *
 * assert 在调试构建里 abort 把"忘填"立刻暴露在调试期, Release 构建定义
 * 了 NDEBUG, assert 编译产物消失, 0 运行时开销.
 *
 * 第三种策略 "全必填·接口" 由 sensor_base.c 演示 -- 那是另一条独立
 * base 线 (sensor_base.h + sensor_temp.h), 跟 led_base 不混.
 * 见 ch14 § 14.4.
 *
 * 见 ch14 § 14.2 / § 14.3 / § 14.7.1 (release 构建里的双层防线).
 */

#include "led_base.h"
#include <assert.h>
#include <stdio.h>

int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops)
{
	if (!me || !name || !ops)
		return -1;

	me->ops   = ops;
	me->name  = name;
	me->is_on = false;

	printf("  [base] \"%s\" common init done, ops=%p\n",
	       name, (const void *)ops);
	return 0;
}

/* ============== 父类统一接口 (必填 + 选填) ============== */

int led_on(struct led_base *me)
{
	if (!me)
		return -1;

	/* 必填: on 是 LED 的核心能力, 不实现等于这个对象无效.
	 * assert 在调试构建里 abort, 告诉你哪个文件哪一行触发;
	 * Release 构建 (-DNDEBUG) 下 assert 整行消失, 零运行时开销.
	 * 这就是 C 模拟"纯虚函数"的等价物.
	 */
	assert(me->ops && me->ops->on &&
	       "led_on: subclass must implement on()");
	return me->ops->on(me);
}

int led_off(struct led_base *me)
{
	if (!me)
		return -1;

	assert(me->ops && me->ops->off &&
	       "led_off: subclass must implement off()");
	return me->ops->off(me);
}

int led_set_brightness(struct led_base *me, uint8_t brightness)
{
	if (!me || !me->ops)
		return -1;

	/* 选填: set_brightness 不是每种 LED 都需要实现 (GPIO 只有
	 * 开/关, 没有亮度概念). 如果让这一项也走 assert 必填, 每个
	 * 不支持调光的子类得写一个空函数, 烦.
	 *
	 * 更好的做法: 父类的统一接口里给一个"默认行为", 子类没填就
	 * 走默认. ops 表本身从来不改, 处理 NULL 的责任落在父类.
	 * 这就是 C 模拟"带默认行为的虚函数"的等价物, 子类可以覆写
	 * 也可以不覆写.
	 */
	if (!me->ops->set_brightness) {
		/* 默认行为: 这种 LED 不支持调光, 安静跳过 */
		printf("  [%s] no dimming support, skip (brightness=%u)\n",
		       me->name, (unsigned)brightness);
		return 0;
	}
	return me->ops->set_brightness(me, brightness);
}
