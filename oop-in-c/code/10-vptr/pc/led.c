/* SPDX-License-Identifier: MIT */
/**
 * @file  led.c
 * @brief 子类 init 把 ops 表交给 base, 调用方走 me->ops->on(me)
 *
 * @details
 * 子类 init (led_gpio_init / led_pwm_init) 把对应的 const ops 表地址
 * 作为常量传给 led_base_init, base 把它存到 me->ops 字段.
 *
 * 调用方拿到 base 指针后直接 me->ops->on(me) -- 那张表是跟着 me
 * 自己跑的, 不用调用方再传一次.
 *
 * 实现层 (gpio_on / pwm_on / ...) 接 struct led_base *, 函数体里
 * 用强转 (struct led_gpio *)me 回到子类拿 pin / channel 等硬件
 * 字段. 这一招的前提是 base 在子类的第 0 偏移 -- C 标准保证 struct
 * 第一个字段地址就是 struct 自身地址, 强转回去能拿到同一块内存.
 */

#include "led.h"
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
 * const + 全局 -> 编译期 designated initializer 把每个字段填好 ->
 * 链接器把这两个对象放进 .rodata 段 -> 运行时只读, 改写直接 SIGSEGV
 * 或 MCU HardFault. 这一层防御是工业代码的硬要求, 防止运行时被
 * 误改成野指针.
 *
 * gpio_xxx / pwm_xxx 都声明成 static -- 它们是实现细节, 应用层
 * 不该直接调. ops 表是它们对外的唯一入口. static 隐藏的不是数据,
 * 是 API 表面.
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

/*
 * ---- test_led 通用测试函数 ----
 *
 * 接 base 指针就够了. ops 表跟着 me 自己跑, 函数体里直接走
 * me->ops->on(me). 同一份 test_led, 不同 LED 不同行为.
 *
 * 三个函数指针的 NULL check 不能少: 某种 LED 不支持 toggle
 * 时 ops->toggle 可能没填, 调用前必须查.
 */
int test_led(struct led_base *me)
{
	if (!me || !me->ops ||
	    !me->ops->on || !me->ops->off || !me->ops->toggle)
		return -1;

	printf("  [test] open ...\n");
	me->ops->on(me);
	printf("  [test] toggle ...\n");
	me->ops->toggle(me);
	printf("  [test] close ...\n");
	me->ops->off(me);
	return 0;
}
