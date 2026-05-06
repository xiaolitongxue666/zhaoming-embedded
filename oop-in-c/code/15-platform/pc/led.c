/* SPDX-License-Identifier: MIT */
/*
 * led.c - LED 驱动层实现
 *
 * 注意：这一层永远只调 platform 层的封装函数（platform_gpio_write /
 * platform_gpio_init 等，在 common/platform.h 声明）。从来不直接碰
 * platform_ops 字段、不 #include platform_ops.h 做 dispatch（这里
 * #include 它只是为了 platform_name() 在 printf 里打印当前平台名）。
 *
 * 切换平台的能力由 platform 层内部完成（platform_dispatch.c 的
 * g_platform_ops 指针），驱动层一字不知。这一层在 PC、STM32、Linux
 * 上都能编译运行，源码 0 修改。
 *
 * 这是工业代码里"对外稳定、对内可换"的纪律：上层永远调封装函数，
 * 内部分发是 platform 层的实现细节。见 ch15 § 15.5 + § 15.7。
 */

#include "led.h"
#include "container_of.h"
#include "platform.h"
#include "platform_ops.h"
#include <assert.h>
#include <stdio.h>

/* ============== 父类统一接口（必填 + 选填） ============== */

int led_on(struct led_base *me)
{
	if (!me)
		return -1;
	/* on 是必填: 子类必须实现. 调试构建里 assert 抓到忘填的子类
	 * 立刻 abort 给行号; Release 构建定义 NDEBUG 后 assert 整行消失,
	 * 零运行时开销. */
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
		printf("  [%s] no dimming, skip\n", me->name);
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
	printf("  [%s] led_on -> platform=%s\n", me->name, platform_name());
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, !self->on_level);
	me->is_on = false;
	return 0;
}

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
	printf("  [%s] led_on -> platform=%s, pwm ch%u duty=%u%%\n",
	       me->name, platform_name(),
	       (unsigned)self->channel, (unsigned)self->duty);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	printf("  [%s] led_off -> platform=%s, pwm ch%u duty=0%%\n",
	       me->name, platform_name(), (unsigned)self->channel);
	me->is_on = false;
	return 0;
}

static int pwm_set_brightness(struct led_base *me, uint8_t brightness)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	if (brightness > 100)
		brightness = 100;
	self->duty = brightness;
	printf("  [%s] set_brightness -> platform=%s, pwm ch%u duty=%u%%\n",
	       me->name, platform_name(),
	       (unsigned)self->channel, (unsigned)brightness);
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
	printf("  [%s] led_on -> platform=%s, i2c bus%u addr=0x%02X\n",
	       me->name, platform_name(),
	       (unsigned)self->bus, (unsigned)self->addr);
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	printf("  [%s] led_off -> platform=%s, i2c bus%u addr=0x%02X\n",
	       me->name, platform_name(),
	       (unsigned)self->bus, (unsigned)self->addr);
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
