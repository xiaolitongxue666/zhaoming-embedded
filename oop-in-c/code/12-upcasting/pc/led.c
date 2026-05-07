/* SPDX-License-Identifier: MIT */
/**
 * @file  led.c
 * @brief 父类统一接口 + 三种子类实现 (GPIO / PWM / I2C)
 *
 * @details
 * 父类的 led_on / led_off 走 ops 表把调用送到子类实现. 子类实现里
 * 第一行 "(struct led_xxx *)me" 强转回子类是简单写法, 前提是 base
 * 在子类第 0 偏移.
 *
 * 子类 init 第一行调 led_base_init (在 led_base.c), 把对应的 ops 表
 * 当常量参数传过去. base 把 ops / name / is_on 一次填好, 子类只管
 * 自己的硬件资源 (pin / channel / addr) 和外设初始化调用.
 *
 * 子类调用 platform 层的 platform_gpio_write / platform_gpio_init 等
 * 封装函数 (signature 跟 ch01 起一字不变). 应用层和驱动层永远只见
 * 普通函数, 不直接碰 ops 字段.
 */

#include "led.h"
#include "platform.h"
#include <stdio.h>

/* ============== 父类统一接口 ============== */

int led_on(struct led_base *me)
{
	if (!me || !me->ops || !me->ops->on)
		return -1;
	return me->ops->on(me);
}

int led_off(struct led_base *me)
{
	if (!me || !me->ops || !me->ops->off)
		return -1;
	return me->ops->off(me);
}

/*
 * ============== 子类一: GPIO LED ==============
 *
 * 实现层接 struct led_base *me. 第一行 (struct led_gpio *)me 强转
 * 回子类拿到 pin 字段. 合法因为 base 在 led_gpio 的第 0 字段
 * (向上转型不变量, 见 ch12 § 12.2).
 *
 * 三种 ops 表全部 static 修饰 + 文件作用域: gpio_ops / pwm_ops /
 * i2c_ops 不暴露给应用层, 应用层只能通过子类 init 间接挂上.
 */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;     /* 反推回子类 */
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	printf("  [%s] GPIO Pin%u ON\n", me->name, (unsigned)self->pin);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	platform_gpio_write(self->pin, !self->on_level);
	me->is_on = false;
	printf("  [%s] GPIO Pin%u OFF\n", me->name, (unsigned)self->pin);
	return 0;
}

static const struct led_ops gpio_ops = {
	.on  = gpio_on,
	.off = gpio_off,
};

/*
 * 子类构造函数 - 第一行调 led_base_init 把 ops 表交给父类,
 * 之后填子类自己的硬件资源, 最后调 platform 层把外设拉起来.
 *
 * platform_gpio_write(pin, !on_level) 在 init 末尾把灯先关掉
 * (避免上电瞬间灯就亮个莫名其妙).
 */
int led_gpio_init(struct led_gpio *me, const char *name,
                  uint8_t pin, bool on_level)
{
	int rc;
	if (!me)
		return -1;
	rc = led_base_init(&me->base, name, &gpio_ops);
	if (rc != 0)
		return rc;
	me->pin = pin;
	me->on_level = on_level;
	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, !on_level);    /* 上电先关灯 */
	return 0;
}

/* ============== 子类二: PWM LED ============== */

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	printf("  [%s] PWM ch%u duty=%u%%\n",
	       me->name, (unsigned)self->channel, (unsigned)self->duty);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	printf("  [%s] PWM ch%u duty=0%%\n",
	       me->name, (unsigned)self->channel);
	me->is_on = false;
	return 0;
}

static const struct led_ops pwm_ops = {
	.on  = pwm_on,
	.off = pwm_off,
};

int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty)
{
	int rc;
	if (!me)
		return -1;
	if (duty > 100)
		return -2;
	rc = led_base_init(&me->base, name, &pwm_ops);
	if (rc != 0)
		return rc;
	me->channel = channel;
	me->duty = duty;
	return 0;
}

/* ============== 子类三: I2C 扩展芯片 LED ============== */

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	printf("  [%s] I2C bus%u addr=0x%02X reg=0x01\n",
	       me->name, (unsigned)self->bus, (unsigned)self->addr);
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	printf("  [%s] I2C bus%u addr=0x%02X reg=0x00\n",
	       me->name, (unsigned)self->bus, (unsigned)self->addr);
	me->is_on = false;
	return 0;
}

static const struct led_ops i2c_ops = {
	.on  = i2c_on,
	.off = i2c_off,
};

int led_i2c_init(struct led_i2c *me, const char *name,
                 uint8_t bus, uint8_t addr)
{
	int rc;
	if (!me)
		return -1;
	rc = led_base_init(&me->base, name, &i2c_ops);
	if (rc != 0)
		return rc;
	me->bus = bus;
	me->addr = addr;
	return 0;
}
