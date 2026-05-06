/* SPDX-License-Identifier: MIT */
/**
 * @file  led.c
 * @brief 子类 init + 子类实现层 (gpio_on / pwm_on / ...)
 *
 * @details
 * 子类 init 把对应的 const ops 表交给 base init, 一次填好.
 *
 * 子类实现 (gpio_on / gpio_off / gpio_toggle / pwm_on / ...) 函数
 * 签名都是 (struct led_base *me) -- 因为父类统一接口 led_on(base)
 * 透过 ops 指针跳过来时, 拿到的就是 base 指针.
 *
 * 函数体里 (struct led_gpio *)me 强转回各自子类拿硬件字段
 * (pin / channel). 这一招的前提是 base 在子类的第 0 偏移: C 标准
 * 保证 struct 第一个字段地址等于 struct 自身地址, 强转回去拿到
 * 同一块内存.
 */

#include "led.h"
#include "platform.h"
#include <stdio.h>

/* 子类 init: gpio */
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

/* 子类 init: pwm */
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

/* ---- GPIO 实现 ---- */

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

/* ---- PWM 实现 ---- */

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
 * const + 全局: 落 .rodata, 全程序唯一一份, 100 颗同类型 LED 共享
 * 同一张表. 静态实现函数全部 static, 只能通过 ops 表对外暴露.
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
