/* SPDX-License-Identifier: MIT */
/*
 * led.c - 子类层 (ch15 完整版, 风格 A)
 *
 * 这一份只负责子类: 三种硬件实现 (gpio_xxx / pwm_xxx / i2c_xxx) +
 * 三个子类 init (led_gpio_init / led_pwm_init / led_i2c_init).
 *
 * 父类统一接口 (led_on / led_off / led_set_brightness) 和共有 init
 * (led_base_init) 都在 led_base.c, 子类 init 第一行调 led_base_init
 * 把对应的 const ops 表交给父类一次填好.
 *
 * 子类实现层 (gpio_on / gpio_off / pwm_on / ...) 函数签名都是
 * (struct led_base *me) -- 父类统一接口透过 ops 跳过来时, 拿到的是
 * base 指针. 第一行用 container_of 反推回子类拿硬件字段
 * (pin / channel / addr). 见 ch13 § 13.5 三步宏 container_of.
 *
 * 这一层永远只调 platform 层的封装函数 (platform_gpio_init / write,
 * 在 common/platform.h 声明), 从来不直接碰寄存器. 同一份 led.c
 * 在 PC、STM32、Linux 上都能编译运行, 源码 0 修改.
 *
 * 见 ch15 § 15.3 子类层.
 */

#include "led.h"
#include "container_of.h"
#include "platform.h"
#include <stdio.h>

/* ============== 子类一: GPIO LED ============== */

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

/* set_brightness 故意不填, GPIO 不支持调光, 走父类默认行为 */
static const struct led_ops gpio_ops = {
	.on  = gpio_on,
	.off = gpio_off,
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
	platform_gpio_write(pin, !on_level);    /* 上电先关灯 */
	return 0;
}

/* ============== 子类二: PWM LED ============== */

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

/* ============== 子类三: I2C 扩展芯片 LED ============== */

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

/* set_brightness 故意不填, I2C 这一路只控开/关, 走父类默认行为 */
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
	me->bus  = bus;
	me->addr = addr;
	return 0;
}
