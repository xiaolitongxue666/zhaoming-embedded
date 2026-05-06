/* SPDX-License-Identifier: MIT */
/*
 * led.c - LED 驱动层实现
 *
 * 这一层永远只调 platform 层的封装函数 (platform_gpio_write /
 * platform_gpio_init 等, 在 common/platform.h 声明), 从来不直接碰寄存器,
 * 也不知道 PC / STM32 / Linux 各自怎么实现.
 *
 * 同一份 led.c 在 PC、STM32、Linux 上都能编译运行, 源码 0 修改. 上层 (app.c)
 * 调的全部是 led_base * 句柄, 下层 (platform_*.c) 提供同样四个函数,
 * led 这一层处在中间, 只关心"GPIO 能写吗、能调亮度吗、能跑 I2C 吗"这种
 * 抽象能力.
 *
 * 见 ch15 § 15.2 父类层 + § 15.3 子类层.
 */

#include "led.h"
#include "container_of.h"
#include "platform.h"
#include <assert.h>
#include <stdio.h>

/* ============== 父类统一接口 (必填 + 选填) ============== */

int led_on(struct led_base *me)
{
	if (!me)
		return -1;
	/* on 是必填: 子类必须实现. 调试构建里 assert 抓到忘填的子类
	 * 立刻 abort 给行号; Release 构建定义 NDEBUG 后 assert 整行消失,
	 * 零运行时开销. 见 ch14 § 14.2. */
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
	if (!me->ops->set_brightness) {
		/* 选填: GPIO 灯没有调光能力, 子类不填 set_brightness 就走
		 * 父类的默认行为 (打印一行 "no dimming, skip"), 不崩.
		 * 见 ch14 § 14.3. */
		printf("  [%s] no dimming, skip (brightness=%u)\n",
		       me->name, (unsigned)brightness);
		return 0;
	}
	return me->ops->set_brightness(me, brightness);
}

/* ============== GPIO 子类 ============== */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	printf("  [%s] led_on  -> GPIO Pin%u\n",
	       me->name, (unsigned)self->pin);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, !self->on_level);
	me->is_on = false;
	printf("  [%s] led_off -> GPIO Pin%u\n",
	       me->name, (unsigned)self->pin);
	return 0;
}

/* set_brightness 故意不填, GPIO 不支持调光 (走父类默认行为) */
static const struct led_ops gpio_ops = {
	.on  = gpio_on,
	.off = gpio_off,
};

void led_gpio_init(struct led_gpio *me, const char *name,
		   uint8_t pin, bool on_level)
{
	me->base.ops   = &gpio_ops;
	me->base.name  = name;
	me->base.is_on = false;
	me->pin        = pin;
	me->on_level   = on_level;
	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
}

/* ============== PWM 子类 ============== */

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	printf("  [%s] led_on  -> PWM ch%u duty=%u%%\n",
	       me->name, (unsigned)self->channel, (unsigned)self->duty);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	printf("  [%s] led_off -> PWM ch%u duty=0%%\n",
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
	printf("  [%s] set_brightness -> PWM ch%u duty=%u%%\n",
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

/* ============== I2C 子类 ============== */

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	printf("  [%s] led_on  -> I2C bus%u addr=0x%02X\n",
	       me->name, (unsigned)self->bus, (unsigned)self->addr);
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	printf("  [%s] led_off -> I2C bus%u addr=0x%02X\n",
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
