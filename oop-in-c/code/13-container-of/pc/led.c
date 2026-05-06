/* SPDX-License-Identifier: MIT */
/*
 * led.c - 子类实现里用 container_of 反推自己
 *
 * 和 ch12 的差别只有一处：(struct led_xxx *)me 强转换成
 *   container_of(me, struct led_xxx, base);
 *
 * GPIO 子类的 base 故意不在第一个位置，container_of 照样对——这是
 * 整个 ch13 想证明的核心。强转那一招要求 base 必须在 offset 0，
 * container_of 没有这条限制：它在编译期算出 base 的真实偏移、运行时
 * 一条减法指令就还原回外层 struct 起点。
 *
 * 见 ch13 § 13.5 / § 13.6。
 */

#include "led.h"
#include "container_of.h"
#include "platform.h"
#include <assert.h>
#include <stdio.h>

/* ============== 父类统一接口 ============== */

int led_on(struct led_base *me)
{
	if (!me)
		return -1;
	/* on 是 LED 的核心能力, 子类必须实现. 调试构建里 assert 抓到
	 * 忘填的子类立刻 abort, Release 构建定义 NDEBUG 后 assert 整行
	 * 消失, 0 运行时开销. */
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
	/* set_brightness 是选填字段, 子类没实现就走父类默认行为
	 * (这里默认 = 安静返回 0). */
	if (!me->ops->set_brightness)
		return 0;
	return me->ops->set_brightness(me, brightness);
}

/* ============== 子类一：GPIO LED ============== */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	printf("  [%s] GPIO Pin%u ON (magic=0x%04X)\n",
	       me->name, (unsigned)self->pin, (unsigned)self->magic);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, !self->on_level);
	me->is_on = false;
	printf("  [%s] GPIO Pin%u OFF\n", me->name, (unsigned)self->pin);
	return 0;
}

static const struct led_ops gpio_ops = {
	.on  = gpio_on,
	.off = gpio_off,
};

void led_gpio_init(struct led_gpio *me, const char *name,
		   uint8_t pin, bool on_level)
{
	me->magic       = 0xCAFE;
	me->base.ops    = &gpio_ops;
	me->base.name   = name;
	me->base.is_on  = false;
	me->pin         = pin;
	me->on_level    = on_level;

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, !on_level);
}

/* ============== 子类二：PWM LED ============== */

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	printf("  [%s] PWM ch%u duty=%u%%\n",
	       me->name, (unsigned)self->channel, (unsigned)self->duty);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	printf("  [%s] PWM ch%u duty=0%%\n",
	       me->name, (unsigned)self->channel);
	me->is_on = false;
	return 0;
}

static int pwm_set_brightness(struct led_base *me, uint8_t brightness)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	if (brightness > 100)
		brightness = 100;
	self->duty = brightness;
	printf("  [%s] PWM ch%u duty=%u%%\n",
	       me->name, (unsigned)self->channel, (unsigned)brightness);
	me->is_on = (brightness > 0);
	return 0;
}

static const struct led_ops pwm_ops = {
	.on             = pwm_on,
	.off            = pwm_off,
	.set_brightness = pwm_set_brightness,
};

void led_pwm_init(struct led_pwm *me, const char *name,
		  uint8_t channel, uint8_t duty)
{
	me->base.ops   = &pwm_ops;
	me->base.name  = name;
	me->base.is_on = false;
	me->channel    = channel;
	me->duty       = duty;
}

/* ============== 子类三：I2C LED ============== */

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	printf("  [%s] I2C bus%u addr=0x%02X reg=0x01\n",
	       me->name, (unsigned)self->bus, (unsigned)self->addr);
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	printf("  [%s] I2C bus%u addr=0x%02X reg=0x00\n",
	       me->name, (unsigned)self->bus, (unsigned)self->addr);
	me->is_on = false;
	return 0;
}

static const struct led_ops i2c_ops = {
	.on  = i2c_on,
	.off = i2c_off,
};

void led_i2c_init(struct led_i2c *me, const char *name,
		  uint8_t bus, uint8_t addr)
{
	me->base.ops   = &i2c_ops;
	me->base.name  = name;
	me->base.is_on = false;
	me->bus        = bus;
	me->addr       = addr;
}
