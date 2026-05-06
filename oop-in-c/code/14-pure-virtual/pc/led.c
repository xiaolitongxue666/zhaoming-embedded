/* SPDX-License-Identifier: MIT */
/*
 * led.c - 父类统一接口实现（必填 + 选填）
 *
 * 关键点："谁来检查 NULL"是这一章的核心抉择。
 * 不在 ops 表本身做手脚（ops 字段类型不变、还是普通函数指针），
 * 把"NULL 不允许"的纪律放到父类统一接口里——assert 在调试构建里
 * abort 把"忘填"立刻暴露在调试期，Release 构建定义了 NDEBUG，
 * assert 编译产物消失，0 开销。
 *
 *   led_on / led_off：必填。ops 没填或为 NULL，立刻 assert 报错并返回错误码。
 *                    对应 C++ 的纯虚函数（virtual void f() = 0;）。
 *   led_set_brightness：选填。ops 没填，安静走父类默认行为返回 0。
 *                    对应 C++ 的虚函数（父类提供默认实现）。
 *
 *   sensor_xxx 三件套：全必填，每一个都 assert。这是接口（interface）的形态，
 *                    合同里每一项都不能少，对应 Java interface / C++ 全纯虚 class。
 *
 * 见 ch14 § 14.2 / 14.3 / 14.4 + § 14.5 NULL 函数指针 = 函数指针强制不能为 NULL。
 */

#include "led.h"
#include "container_of.h"
#include "platform.h"
#include <assert.h>
#include <stdio.h>

/* ============== 父类统一接口（必填 + 选填混合） ============== */

int led_on(struct led_base *me)
{
	if (!me)
		return -1;

	/* 必填：on 是 LED 的核心能力，不实现等于这个对象无效。
	 * assert 在调试构建里 abort，告诉你哪个文件哪一行触发；
	 * Release 构建（-DNDEBUG）下 assert 整行消失，零运行时开销。
	 * 这就是 C 模拟"纯虚函数"的等价物。
	 */
	assert(me->ops && me->ops->on && "led_on: subclass must implement on()");
	return me->ops->on(me);
}

int led_off(struct led_base *me)
{
	if (!me)
		return -1;

	assert(me->ops && me->ops->off && "led_off: subclass must implement off()");
	return me->ops->off(me);
}

int led_set_brightness(struct led_base *me, uint8_t brightness)
{
	if (!me || !me->ops)
		return -1;

	/* 选填：set_brightness 不是每种 LED 都需要实现（GPIO 只有开/关，
	 * 没有亮度概念）。如果让这一项也走 assert 必填，每个不支持调光的
	 * 子类得写一个空函数，烦。
	 *
	 * 更好的做法：父类的统一接口里给一个"默认行为"——子类没填就走默认。
	 * ops 表本身从来不改，处理 NULL 的责任落在父类。这就是 C 模拟
	 * "带默认行为的虚函数"的等价物，子类可以覆写也可以不覆写。
	 */
	if (!me->ops->set_brightness) {
		/* 默认行为：这种 LED 不支持调光，安静跳过 */
		printf("  [%s] no dimming support, skip (brightness=%u)\n",
		       me->name, (unsigned)brightness);
		return 0;
	}
	return me->ops->set_brightness(me, brightness);
}

/* ============== GPIO 子类：只填 on / off ============== */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	printf("  [%s] GPIO Pin%u ON\n", me->name, (unsigned)self->pin);
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

/* set_brightness 故意不填，GPIO 不支持调光 */
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

/* ============== PWM 子类：三件套全填 ============== */

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

/* ============== sensor 接口：三件套全必填 ==============
 *
 * 接口（interface）= 全部必填的合同。read / calibrate / self_test
 * 每一项都不能少。一个 sensor 不能读、或者不能校准、或者不能自检，
 * 它就不算 sensor。这是 C 模拟"纯接口"的形态，对应 C++ 全纯虚 class /
 * Java interface。每一个统一接口函数都 assert，零容忍。
 */

int sensor_read(struct sensor *me, int32_t *out)
{
	if (!me || !out)
		return -1;
	assert(me->ops && me->ops->read &&
	       "sensor.read is part of the interface contract");
	return me->ops->read(me, out);
}

int sensor_calibrate(struct sensor *me)
{
	if (!me)
		return -1;
	assert(me->ops && me->ops->calibrate &&
	       "sensor.calibrate is part of the interface contract");
	return me->ops->calibrate(me);
}

int sensor_self_test(struct sensor *me)
{
	if (!me)
		return -1;
	assert(me->ops && me->ops->self_test &&
	       "sensor.self_test is part of the interface contract");
	return me->ops->self_test(me);
}
