/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.c - LED GPIO 子类实现.
 *
 * 子类只调 platform_pin 封装函数, 永远不直接碰 GPIO 寄存器, 也不 include
 * 厂家 HAL 头文件. 跨芯片移植时这一份代码 0 改动, 只换 platform_pin
 * 子类即可 (STM32 HAL / libgpiod / sysfs / 模拟). 见第 15 章.
 */

#include <stddef.h>

#include "led_gpio.h"
#include "platform_pin.h"

static platform_err_t _led_gpio_on(struct led_base *me);
static platform_err_t _led_gpio_off(struct led_base *me);

/* static const ops 表给所有 led_gpio 实例共享, 落 .rodata. */
static const struct led_ops led_gpio_ops = {
	.on             = _led_gpio_on,
	.off            = _led_gpio_off,
	.set_brightness = NULL,    /* GPIO LED 不支持调亮度, 走父类默认 no-op */
};

platform_err_t led_gpio_init(struct led_gpio *me, const char *name,
                             const char *pin_name, bool active_high)
{
	platform_err_t ret;
	int32_t        pin;

	if ((NULL == me) || (NULL == pin_name)) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	pin = platform_pin_get(pin_name);
	if (pin < 0) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	platform_pin_mode(pin, PIN_MODE_OUTPUT);
	platform_pin_write(pin, active_high ? PIN_LOW : PIN_HIGH);

	me->pin         = pin;
	me->active_high = active_high;

	ret = led_base_init(&me->base, name, &led_gpio_ops);

exit:
	return ret;
}

/* 子类实现: me 形参类型是基类指针, 函数体里直接 cast 回子类指针拿私有
 * 字段. 因为 led_base 在 led_gpio 的第一字段, 两者地址重合, cast 是 0
 * 偏移; 基类不在第一字段时要用 container_of (见第 13 章). */
static platform_err_t _led_gpio_on(struct led_base *me)
{
	struct led_gpio *gpio = (struct led_gpio *)me;

	platform_pin_write(gpio->pin, gpio->active_high ? PIN_HIGH : PIN_LOW);
	return PLATFORM_EOK;
}

static platform_err_t _led_gpio_off(struct led_base *me)
{
	struct led_gpio *gpio = (struct led_gpio *)me;

	platform_pin_write(gpio->pin, gpio->active_high ? PIN_LOW : PIN_HIGH);
	return PLATFORM_EOK;
}
