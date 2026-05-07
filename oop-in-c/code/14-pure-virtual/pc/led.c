/* SPDX-License-Identifier: MIT */
/*
 * led.c - 父类统一接口 (三种策略) + 子类实现
 *
 * "谁来检查 NULL"是这一章的核心抉择. 不在 ops 表本身做手脚 (ops 字段
 * 类型不变, 还是普通函数指针), 把"NULL 不允许"的纪律放到父类统一接口里:
 *
 *   led_on / led_off    -> 必填. ops 没填或为 NULL, assert 报错并返回错误码.
 *                         对应 C++ 纯虚函数 (virtual void f() = 0;).
 *   led_set_brightness  -> 选填. ops 没填, 父类的统一接口走默认行为返回 0.
 *                         对应 C++ 带默认实现的虚函数.
 *
 * assert 在调试构建里 abort 把"忘填"立刻暴露在调试期, Release 构建定义
 * 了 NDEBUG, assert 编译产物消失, 0 运行时开销.
 *
 * 子类 init (led_gpio_init / led_pwm_init) 第一行调 led_base_init, 把
 * 对应的 const ops 表作为常量传进来. 子类实现 (gpio_on / pwm_on / ...)
 * 接 struct led_base *, 第一行用 container_of 反推回子类指针拿 pin /
 * channel 等硬件字段. container_of 是 ch13 学的 Linux 内核宏, 编译期
 * 算偏移, 运行时一条减法指令, 0 开销.
 *
 * 见 ch14 § 14.2 / § 14.3 / § 14.7.1 (release 构建里的双层防线).
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

	/* 必填: on 是 LED 的核心能力, 不实现等于这个对象无效.
	 * assert 在调试构建里 abort, 告诉你哪个文件哪一行触发;
	 * Release 构建 (-DNDEBUG) 下 assert 整行消失, 零运行时开销.
	 * 这就是 C 模拟"纯虚函数"的等价物.
	 */
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

	/* 选填: set_brightness 不是每种 LED 都需要实现 (GPIO 只有
	 * 开/关, 没有亮度概念). 如果让这一项也走 assert 必填, 每个
	 * 不支持调光的子类得写一个空函数, 烦.
	 *
	 * 更好的做法: 父类的统一接口里给一个"默认行为", 子类没填就
	 * 走默认. ops 表本身从来不改, 处理 NULL 的责任落在父类.
	 * 这就是 C 模拟"带默认行为的虚函数"的等价物, 子类可以覆写
	 * 也可以不覆写.
	 */
	if (!me->ops->set_brightness) {
		/* 默认行为: 这种 LED 不支持调光, 安静跳过 */
		printf("  [%s] no dimming support, skip (brightness=%u)\n",
		       me->name, (unsigned)brightness);
		return 0;
	}
	return me->ops->set_brightness(me, brightness);
}

/* ============== GPIO 子类: 只填 on / off ============== */

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

/*
 * GPIO 子类的 ops 表: 只填 on / off, set_brightness 故意不填.
 * C 标准下静态存储未显式初始化的字段被零初始化, 所以
 * gpio_ops.set_brightness 是 NULL. 这正是选填策略要演示的情形.
 *
 * static + const + designated initializer 的组合让这张表落进 .rodata 段:
 *   - MCU 上烧到 Flash, 完全不占 RAM
 *   - 100 颗同类型 GPIO LED 共享同一张表
 *   - 链接期不可改, 防止运行时被踩成野指针
 */
static const struct led_ops gpio_ops = {
	.on  = gpio_on,
	.off = gpio_off,
	/* set_brightness 故意不填 -- GPIO 灯没有亮度概念 */
};

int led_gpio_init(struct led_gpio *me, const char *name,
                  uint8_t pin, bool on_level)
{
	int rc;
	if (!me)
		return -1;

	rc = led_base_init(&me->base, name, &gpio_ops);
	if (rc != 0)
		return rc;

	me->pin      = pin;
	me->on_level = on_level;

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, !on_level);     /* 上电先关灯 */
	return 0;
}

/* ============== PWM 子类: 三件套全填 ============== */

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
	me->duty    = duty;
	return 0;
}
