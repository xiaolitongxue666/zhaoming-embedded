/* SPDX-License-Identifier: MIT */
/**
 * @file  led.c
 * @brief 多态完整版 - dispatch 三件齐, "同名函数不同行为"落地
 *
 * @details
 * 走到本章, 一行 led_on(me) 就能让红灯走 gpio_on, 蓝灯走 pwm_on,
 * 绿灯走 gpio_on... 三种实现按 me->ops 自动分发. 应用层把它们丢
 * 进同一个数组循环跑, 输出三种不同行为. 这就是多态.
 *
 * 实现层全部接 (struct led_base *me), 内部强转回各自子类取硬件
 * 字段 (pin / channel). 所有 platform 操作走 platform_gpio_xxx
 * 封装函数, 跟 ch01 起一字不变 -- platform 层内部自己走 ops
 * dispatch (见 platform_dispatch.c), led 这一层一字不知.
 *
 * 加新 LED 类型怎么改 (ch11 § 11.6.3 开闭原则):
 *   1) 写一个 const struct led_ops led_ops_xxx 填 3 个函数
 *   2) 写一个 led_xxx_init 调 led_base_init(&me->base, name, &led_ops_xxx)
 *   3) 完了
 * led_on / led_off / led_toggle / struct led_base / 应用层一行不改.
 */

#include "led.h"
#include "platform.h"
#include <assert.h>
#include <stdio.h>

#define GPIO_MODE_OUTPUT    0

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin)
{
	int rc;
	if (!me)
		return -1;
	rc = led_base_init(&me->base, name, &led_ops_gpio);
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
	rc = led_base_init(&me->base, name, &led_ops_pwm);
	if (rc != 0)
		return rc;
	me->channel = channel;
	me->duty = duty;
	printf("  [PWM] sub-class init done (channel=%u, duty=%u)\n",
	       (unsigned)channel, (unsigned)duty);
	return 0;
}

/*
 * ============== 应用层 dispatch 接口 ==============
 *
 * me->ops->on(me) 是多态调用的核心. 同一行代码, 因为 me 不同,
 * 两次 LDR 之后落在不同函数 (红灯走 gpio_on, 蓝灯走 pwm_on).
 *
 * on / off / toggle 都是 LED 的核心能力, 子类必须实现. 调试构建里
 * 用 assert 抓到忘填的子类立刻 abort, Release 构建定义 NDEBUG 后
 * assert 整行消失, 零运行时开销.
 */
int led_on(struct led_base *me)
{
	if (!me)
		return -1;
	assert(me->ops && me->ops->on &&
	       "led_on: subclass must implement on()");
	me->is_on = true;
	return me->ops->on(me);
}

int led_off(struct led_base *me)
{
	if (!me)
		return -1;
	assert(me->ops && me->ops->off &&
	       "led_off: subclass must implement off()");
	me->is_on = false;
	return me->ops->off(me);
}

int led_toggle(struct led_base *me)
{
	if (!me)
		return -1;
	assert(me->ops && me->ops->toggle &&
	       "led_toggle: subclass must implement toggle()");
	me->is_on = !me->is_on;
	return me->ops->toggle(me);
}

/* GPIO style */
static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	platform_gpio_write(self->pin, true);
	printf("  [GPIO] \"%s\" ON\n", me->name);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	platform_gpio_write(self->pin, false);
	printf("  [GPIO] \"%s\" OFF\n", me->name);
	return 0;
}

static int gpio_toggle(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	platform_gpio_write(self->pin, me->is_on);
	printf("  [GPIO] \"%s\" toggle -> %s\n",
	       me->name, me->is_on ? "ON" : "OFF");
	return 0;
}

/* PWM style */
static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	printf("  [PWM] \"%s\" ON  (channel %u, duty=%u)\n",
	       me->name, (unsigned)self->channel, (unsigned)self->duty);
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	printf("  [PWM] \"%s\" OFF (channel %u)\n",
	       me->name, (unsigned)self->channel);
	return 0;
}

static int pwm_toggle(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	printf("  [PWM] \"%s\" toggle -> %s (channel %u)\n",
	       me->name, me->is_on ? "ON" : "OFF",
	       (unsigned)self->channel);
	return 0;
}

/*
 * 两张 ops 表 - const 落 .rodata, 全程序共享.
 * 100 颗同类型 LED 共享同一张 12 字节表, RAM 里只多一个 4 字节 vptr.
 * 这就是 Linux 内核 file_operations / net_device_ops 的存储策略.
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
