/* SPDX-License-Identifier: MIT */
/**
 * @file  led.c
 * @brief 父类统一接口 + 三种子类实现 (GPIO / PWM / I2C)
 *
 * @details
 * 父类的 led_on / led_off / led_set_brightness 通过 ops 表分发到
 * 子类实现. 整条调用链 (见 ch12 § 12.8.6):
 *
 *   app:  led_on(g_led_error)
 *           |
 *   base:  return me->ops->on(me);    // 一行胶水, dispatch
 *           |
 *   sub:   static int gpio_on(struct led_base *me) {
 *              struct led_gpio *self = (struct led_gpio *)me;  // 反推回子类
 *              platform_gpio_write(self->pin, self->on_level);
 *              ...
 *          }
 *
 * 应用层从头到尾只见 struct led_base *.
 *
 * 子类实现里第一行 "(struct led_xxx *)me" 强转回子类是简单写法,
 * 前提是 base 在子类第 0 偏移. ch13 会把这一步换成位置无关的
 * container_of, 让 base 不必非得在第一个位置.
 *
 * 子类调用 platform 层的 platform_gpio_write / platform_gpio_init 等
 * 封装函数 (signature 跟 ch01 起一字不变). 这是工业代码"应用层和
 * 驱动层永远只见普通函数, 不直接碰 ops 字段"的标准做法.
 */

#include "led.h"
#include "platform.h"
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

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
	if (!me->ops->set_brightness) {
		/*
		 * 这种 LED 不支持调光 (GPIO / I2C 都没填这个字段),
		 * 安静返回 0 = 成功 + 啥也没做. set_brightness 是选填字段,
		 * 子类没实现就走父类默认行为. ch14 § 14.3 会系统展开"必填 + 选填
		 * + 全必填接口"三种处理策略.
		 */
		return 0;
	}
	return me->ops->set_brightness(me, brightness);
}

/*
 * ============== 子类一: GPIO LED ==============
 *
 * 实现层接 struct led_base *me. 第一行 (struct led_gpio *)me 强转
 * 回子类拿到 pin 字段. 合法因为 base 在 led_gpio 的第 0 字段
 * (向上转型不变量, 见 ch12 § 12.2). ch13 用 container_of 替换这一
 * 步, 让 base 不必非得在第一个位置.
 *
 * 三种 ops 表全部 static 修饰 + 文件作用域: gpio_ops / pwm_ops /
 * i2c_ops 不暴露给应用层, 应用层只能通过子类 init 间接挂上.
 * 见 ch10 § 10.11 工业纪律 "ops 是基类层的实现细节, 不暴露给应用层".
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

/*
 * GPIO 灯不支持调光, set_brightness 字段故意不填.
 * designated initializer 把没列出的字段填为 NULL (C99 § 6.7.8).
 * led_set_brightness 包装函数遇到 NULL 时安静返回 (见上面).
 */
static const struct led_ops gpio_ops = {
	.on  = gpio_on,
	.off = gpio_off,
};

/*
 * 子类构造函数 - 把 ops 表填进 base, 再填子类自己的硬件资源.
 *
 * me->base.ops = &gpio_ops 这一步等价于 C++ 编译器在 led_gpio
 * 对象构造时自动设 vptr = &led_gpio::vtable (见 ch10 § 10.7).
 * 你 C 里手动写, 机器码几乎一字不差.
 *
 * platform_gpio_write(pin, !on_level) 在 init 末尾把灯先关掉
 * (避免上电瞬间灯就亮个莫名其妙). on_level=1 时写 0=低电平=灭,
 * on_level=0 时写 1=高电平=灭.
 */
void led_gpio_init(struct led_gpio *me, const char *name,
		   uint8_t pin, bool on_level)
{
	me->base.ops   = &gpio_ops;
	me->base.name  = name;
	me->base.is_on = false;
	me->pin        = pin;
	me->on_level   = on_level;

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, !on_level);    /* 上电先关灯 */
}

/* ============== 子类二：PWM LED ============== */

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

static int pwm_set_brightness(struct led_base *me, uint8_t brightness)
{
	struct led_pwm *self = (struct led_pwm *)me;
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

/* ============== 子类三：I2C 扩展芯片 LED ============== */

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

void led_i2c_init(struct led_i2c *me, const char *name,
		  uint8_t bus, uint8_t addr)
{
	me->base.ops   = &i2c_ops;
	me->base.name  = name;
	me->base.is_on = false;
	me->bus        = bus;
	me->addr       = addr;
}
