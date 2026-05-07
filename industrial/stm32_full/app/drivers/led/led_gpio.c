/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.c - LED GPIO 子类实现.
 *
 * 子类只调 platform_pin 封装函数, 永远不直接碰 GPIO 寄存器, 也不 include
 * platform_pin 内部头文件. 跨芯片移植时这一份代码 0 改动.
 */

#include <stddef.h>

#include "drivers/led/led_gpio.h"
#include "platform/platform_pin.h"

static platform_err_t _led_gpio_on(struct led_base *me);
static platform_err_t _led_gpio_off(struct led_base *me);

/* static const ops 表给所有 led_gpio 实例共享, 放 Flash. */
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
