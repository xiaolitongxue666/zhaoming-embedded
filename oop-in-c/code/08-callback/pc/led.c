/* SPDX-License-Identifier: MIT */
/**
 * @file  led.c
 * @brief 函数指针当参数 + 回调注册
 *
 * @details
 * test_led(me, on, off): 接受三个参数, 后两个是函数指针. test_led
 *   自己不知道调谁, 调用方告诉它. 这就是 ch08 § 8.2 的"延迟决定".
 *
 * led_on(base) / led_off(base): 走具体 GPIO 实现 (本章子类只有
 *   GPIO 一种), 状态翻转后调 base->on_state_change 通知应用层.
 *   on_state_change 字段为 NULL 时不调 -- 这一步 NULL check 是
 *   工业代码硬规则 (见 ch08 § 8.6.6).
 */

#include "led.h"
#include <stdio.h>

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin)
{
	int rc;
	if (!me)
		return -1;

	rc = led_base_init(&me->base, name);
	if (rc != 0)
		return rc;

	me->pin = pin;
	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, false);
	printf("  [GPIO] sub-class init done (pin=%u)\n", (unsigned)pin);
	return 0;
}

int gpio_on_pull_high(struct led_base *me)
{
	struct led_gpio *self;
	if (!me)
		return -1;
	/*
	 * base 是 struct led_gpio 的第一个字段, C99 § 6.7.2.1 保证
	 * "结构体第一个成员的地址等于结构体本身的地址", 因此 (struct
	 * led_gpio *)me 强转回去拿到的就是子类对象的起点, 可以安全
	 * 访问 self->pin 这种子类字段.
	 *
	 * 这是向下转型 (downcasting). ch10 还会用同样的强转, ch13 才
	 * 引入 container_of 让 base 不必非得在第一个位置.
	 */
	self = (struct led_gpio *)me;
	platform_gpio_write(self->pin, true);
	printf("  [GPIO] \"%s\" ON (pull_high)\n", me->name);
	return 0;
}

int gpio_off(struct led_base *me)
{
	struct led_gpio *self;
	if (!me)
		return -1;
	self = (struct led_gpio *)me;
	platform_gpio_write(self->pin, false);
	printf("  [GPIO] \"%s\" OFF\n", me->name);
	return 0;
}

/*
 * 通用测试工具函数. 见 ch08 § 8.2.
 *
 * test_led 接受 me + 两个函数指针. 函数体不知道也不需要知道
 * 这两个函数具体是哪个文件、哪个实现 -- 它信任调用方传进来的
 * 是合法的 on/off, 按 "open -> hold -> close" 三步跑完.
 *
 * NULL check 三个都不能少: me 是常规, on 和 off 是函数指针,
 * 调用前必须确认非 NULL, 否则 BLX 跳到地址 0 等于 HardFault.
 * 工业硬规则: 所有函数指针调用前都做 NULL check (ch08 § 8.6.6).
 */
int test_led(struct led_base *me,
             int (*on)(struct led_base *me),
             int (*off)(struct led_base *me))
{
	if (!me || !on || !off)
		return -1;

	printf("  [test] open ...\n");
	on(me);                      /* 调谁? 调用方传进来的 on, 不是字段里的 */
	printf("  [test] hold ...\n");
	printf("  [test] close ...\n");
	off(me);
	return 0;
}

/*
 * led_on / led_off - 应用层调用包装.
 *
 * 内部直接走 gpio_on_pull_high / gpio_off (本章子类只有 GPIO 一种,
 * 还没系统化成 ops 表 dispatch -- 那是 ch10/ch11 的事). 重点在最后
 * 几行: 状态翻转后, 如果有人注册了 on_state_change, 就回调一下.
 *
 * led.c 完全不知道回调里要做什么 -- 是发 CAN、打日志、写统计, 还是
 * 啥都不做. 应用层挂什么函数, led.c 只负责"在合适时机替它拨号".
 */
int led_on(struct led_base *me)
{
	int rc;
	if (!me)
		return -1;

	rc = gpio_on_pull_high(me);
	me->is_on = true;
	if (me->on_state_change)        /* NULL check: 没人注册时不调 */
		me->on_state_change(me, true);
	return rc;
}

int led_off(struct led_base *me)
{
	int rc;
	if (!me)
		return -1;

	rc = gpio_off(me);
	me->is_on = false;
	if (me->on_state_change)
		me->on_state_change(me, false);
	return rc;
}
