/* SPDX-License-Identifier: MIT */
/**
 * @file  led.c
 * @brief ops 表落地: 两张 const 表 (gpio / pwm) 落 .rodata, 全程序共享
 *
 * @details
 * ops 表里函数指针的参数类型是 struct led_base *, 不是某个具体子类.
 * 这是因为 ops 表的最终用户是基类层 (ch10 起 led_on(base) 内部走
 * me->ops->on(me)), 所有子类的 ops 表必须类型一致 -- 否则基类层
 * 没法用同一份 dispatch 代码统一调用所有子类.
 *
 * 实现层从 base 反推到子类用强转 (struct led_xxx *)me. 这一招前提
 * 是 base 在子类的第一个字段, ch13 引入 container_of 后才让 base
 * 不必非得在第一个位置. 见 ch10 § 10.8.6 / ch13.
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

int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty)
{
	int rc;
	if (!me)
		return -1;
	if (duty > 100)
		return -2;
	rc = led_base_init(&me->base, name);
	if (rc != 0)
		return rc;
	me->channel = channel;
	me->duty = duty;
	printf("  [PWM] sub-class init done (channel=%u, duty=%u)\n",
	       (unsigned)channel, (unsigned)duty);
	return 0;
}

/* ---- GPIO style 实现 ---- */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	me->is_on = true;
	platform_gpio_write(self->pin, true);
	printf("  [GPIO] \"%s\" ON\n", me->name);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	me->is_on = false;
	platform_gpio_write(self->pin, false);
	printf("  [GPIO] \"%s\" OFF\n", me->name);
	return 0;
}

static int gpio_toggle(struct led_base *me)
{
	if (me->is_on)
		return gpio_off(me);
	return gpio_on(me);
}

/* ---- PWM style 实现 ---- */

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	me->is_on = true;
	printf("  [PWM] \"%s\" ON  (channel %u, duty=%u)\n",
	       me->name, (unsigned)self->channel, (unsigned)self->duty);
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	me->is_on = false;
	printf("  [PWM] \"%s\" OFF (channel %u)\n",
	       me->name, (unsigned)self->channel);
	return 0;
}

static int pwm_toggle(struct led_base *me)
{
	if (me->is_on)
		return pwm_off(me);
	return pwm_on(me);
}

/*
 * ---- 两张 ops 表 ----
 *
 * const + 全局 -> 编译期 designated initializer 把每个字段填好 ->
 * 链接器把这两个对象放进 .rodata 段 -> 运行时只读, 改写直接 SIGSEGV
 * 或 MCU HardFault. 这一层防御是工业代码的硬要求, 防止运行时被
 * 误改成野指针 (见 ch09 § 9.5.3 内存代价节).
 *
 * 注意 designated initializer .on = ... 这种写法 (C99): 不依赖字段
 * 顺序, 哪天调换 struct 字段, 这里不用改. 没列出的字段自动填 0/NULL
 * (C99 § 6.7.8 第 21 段). 见 ch09 § 9.5.1.
 */

const struct led_ops led_ops_gpio = {
	.on     = gpio_on,
	.off    = gpio_off,
	.toggle = gpio_toggle,
};

const struct led_ops led_ops_pwm = {
	.on     = pwm_on,
	.off    = pwm_off,
	.toggle = pwm_toggle,
};

/*
 * ---- test_led 接受 ops 表 ----
 *
 * 参数从 ch08 的 (me, on, off, toggle) 收成 (me, ops). 函数体内
 * 按名字访问 ops->on / ops->off / ops->toggle, 永远不会传反.
 *
 * 三个函数指针 NULL check 都不能少 (ch09 § 9.5.2): 某种 LED 不支持
 * toggle 时 ops->toggle 可能没填, 调用前必须查. ch14 会展开三种
 * 处理策略 (报错 / 空 stub / 纯虚约定).
 */

int test_led(struct led_base *me, const struct led_ops *ops)
{
	if (!me || !ops || !ops->on || !ops->off || !ops->toggle)
		return -1;

	printf("  [test] open ...\n");
	ops->on(me);
	printf("  [test] toggle ...\n");
	ops->toggle(me);
	printf("  [test] close ...\n");
	ops->off(me);
	return 0;
}
