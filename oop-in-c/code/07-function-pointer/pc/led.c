/* SPDX-License-Identifier: MIT */
/**
 * @file  led.c
 * @brief 通过函数指针字段实现"运行时换实现"
 *
 * @details
 * led_gpio_on() 这个对外 API 不变, 但函数体不再写死调谁, 而是
 *   return me->on_func(me);
 * 一行胶水, 把决定权交给"实例自己带的函数指针字段". 谁先填进
 * 字段, 谁就被调到. 见 ch07 § 7.3 "把调谁做成可填的字段".
 *
 * 运行时也能换: main.c 末尾会演示 red_led.on_func = gpio_on_pull_low;
 * 之后 led_gpio_on(&red_led) 就走低有效逻辑 -- led_gpio_on 一行不改.
 *
 * 工业代码硬规则: 所有函数指针调用前都要 NULL check (见下面的
 * if (!me || !me->on_func) return -1; ). 跳到地址 0 执行的代价是
 * Cortex-M HardFault 或 Linux 用户态 SIGSEGV. 见 ch07 § 7.7.3.
 */

#include "led.h"
#include <stdio.h>

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin,
                  int (*on_func)(struct led_gpio *me))
{
	int rc;

	if (!me || !on_func)
		return -1;

	rc = led_base_init(&me->base, name);
	if (rc != 0)
		return rc;

	me->pin = pin;
	me->on_func = on_func;

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, false);

	printf("  [GPIO] Pin%u init done, on_func=%p\n",
	       (unsigned)pin, (void *)(uintptr_t)on_func);
	return 0;
}

int led_gpio_on(struct led_gpio *me)
{
	if (!me || !me->on_func)
		return -1;

	/*
	 * 调谁? me 自己说. led_gpio_on 不写死.
	 * 这一行就是 ch07 的核心: 通过字段间接跳转, 实现运行时绑定.
	 * ARM Cortex-M4 上编译成 LDR + BLX 两条指令, 多 ~28ns @ 168MHz,
	 * 远低于一次 GPIO/I2C 操作本身的延迟 (见 ch07 § 7.7.2).
	 */
	return me->on_func(me);
}

int led_gpio_off(struct led_gpio *me)
{
	if (!me)
		return -1;

	me->base.is_on = false;
	platform_gpio_write(me->pin, false);
	printf("  [GPIO] \"%s\" OFF\n", me->base.name);
	return 0;
}

/*
 * ---- 两种 on_func 实现 ----
 *
 * 这两个函数本身是普通函数, 它们的"地址"就是 struct led_gpio.on_func
 * 字段要存的值. 编译器在 .text 段给每个函数分配起点地址, init 时把
 * 这个地址写进 .bss/.data 上的对象字段. 调用时从字段取地址跳过去.
 *
 * 这就是"号码 vs 拨号"的区别 (见 ch07 § 7.4):
 *   gpio_on_pull_high              -> 函数名当表达式用, 退化为地址 (号码)
 *   me->on_func = gpio_on_pull_high; -> 把号码存进字段
 *   me->on_func(me)                -> 从字段取号码, 拨号
 */

int gpio_on_pull_high(struct led_gpio *me)
{
	if (!me)
		return -1;
	me->base.is_on = true;
	platform_gpio_write(me->pin, true);
	printf("  [GPIO] \"%s\" ON (pull_high)\n", me->base.name);
	return 0;
}

int gpio_on_pull_low(struct led_gpio *me)
{
	if (!me)
		return -1;
	me->base.is_on = true;
	/* 低有效: 拉低才点亮 (有些 LED 共阳极, 引脚拉低才形成回路) */
	platform_gpio_write(me->pin, false);
	printf("  [GPIO] \"%s\" ON (pull_low, active-low)\n", me->base.name);
	return 0;
}
