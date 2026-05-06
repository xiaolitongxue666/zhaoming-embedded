/* SPDX-License-Identifier: MIT */
/**
 * @file  led.c
 * @brief 子类带着自己的 ops, 应用层只见 base 指针
 *
 * @details
 * 子类 init (led_gpio_init / led_pwm_init) 把对应的 const ops 表地址
 * 作为常量传给 led_base_init, base 把它存到 me->ops 字段.
 *
 * 应用层只调 led_on(struct led_base *me), 函数体 me->ops->on(me)
 * ARM Cortex-M4 编译出两条 LDR + 一条 BX:
 *   LDR  r3, [r0, #0]      ; r3 = me->ops      (ops 在 base 偏移 0)
 *   LDR  r3, [r3, #0]      ; r3 = me->ops->on  (on 在 ops 偏移 0)
 *   BX   r3                ; tail-call 跳到 on, r0 还是 me 即第一参数
 * 这就是 C++ 虚函数 vcall 的精确成本: 两次 LDR + 一次跳转,
 * 约 56ns @ 168MHz (见 ch10 § 10.8.4 / ch11 § 11.2).
 *
 * 实现层 (gpio_on / pwm_on / ...) 接 struct led_base *, 用强转
 * (struct led_gpio *)me 回到子类拿 pin / channel 等硬件字段.
 * 这一招前提是 base 在子类的第 0 偏移, ch13 引入 container_of
 * 才让 base 不必非得在第一个位置. 见 ch10 § 10.8.6.
 */

#include "led.h"
#include <assert.h>
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

/*
 * ============== 应用层 dispatch 接口 ==============
 *
 * on / off / toggle 都是 LED 的核心能力, 子类必须实现.
 * me 本身可能传错, 一律 return -1; ops 字段或者具体 op 没填属于
 * 子类作者忘了实现, 调试构建里 assert 立刻 abort 抓到, Release
 * 构建定义 NDEBUG 后 assert 整行消失, 0 运行时开销.
 *
 * 函数体只有一行 me->ops->on(me) 是 dispatch 核心, 这就是多态在
 * C 里的具体落点. 写这一层胶水的两个理由 (见 ch11 § 11.6.1):
 *   1) 统一参数检查, 集中处理
 *   2) API 稳定 - 哪天 dispatch 机制改了 (加日志/hook/trace),
 *      改一个 led_on 函数, 所有应用层调用方不用动
 */
int led_on(struct led_base *me)
{
	if (!me)
		return -1;
	assert(me->ops && me->ops->on &&
	       "led_on: subclass must implement on()");
	me->is_on = true;
	return me->ops->on(me);
}

int led_off(struct led_base *me)
{
	if (!me)
		return -1;
	assert(me->ops && me->ops->off &&
	       "led_off: subclass must implement off()");
	me->is_on = false;
	return me->ops->off(me);
}

int led_toggle(struct led_base *me)
{
	if (!me)
		return -1;
	assert(me->ops && me->ops->toggle &&
	       "led_toggle: subclass must implement toggle()");
	me->is_on = !me->is_on;
	return me->ops->toggle(me);
}

/* ---- GPIO 实现 ---- */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	platform_gpio_write(self->pin, true);
	printf("  [GPIO] \"%s\" ON\n", me->name);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	platform_gpio_write(self->pin, false);
	printf("  [GPIO] \"%s\" OFF\n", me->name);
	return 0;
}

static int gpio_toggle(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	platform_gpio_write(self->pin, me->is_on);
	printf("  [GPIO] \"%s\" toggle -> %s\n",
	       me->name, me->is_on ? "ON" : "OFF");
	return 0;
}

/* ---- PWM 实现 ---- */

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	printf("  [PWM] \"%s\" ON  (channel %u, duty=%u)\n",
	       me->name, (unsigned)self->channel, (unsigned)self->duty);
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	printf("  [PWM] \"%s\" OFF (channel %u)\n",
	       me->name, (unsigned)self->channel);
	return 0;
}

static int pwm_toggle(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	printf("  [PWM] \"%s\" toggle -> %s (channel %u)\n",
	       me->name, me->is_on ? "ON" : "OFF",
	       (unsigned)self->channel);
	return 0;
}

/*
 * 两张 ops 表 - const 落 .rodata, 全程序共享.
 *
 * 100 颗 GPIO LED 共享同一个 12 字节 led_ops_gpio (Flash, 不占 RAM),
 * 每颗 LED 对象里只多 4 字节的 me->ops 指针. 这就是 vtable 的存储
 * 策略: "类一份, 对象只存 vptr". 见 ch09 § 9.5.3 / ch10 § 10.11.
 *
 * gpio_xxx / pwm_xxx 都声明成 static -- 它们是基类层 dispatch 的
 * 实现细节, 应用层不该直接调. ops 表是它们对外的唯一入口.
 * static 隐藏的不是数据, 是 API 表面.
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
