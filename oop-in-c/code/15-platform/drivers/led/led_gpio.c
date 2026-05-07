/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.c - LED GPIO 子类实现.
 *
 * 子类只调 platform_pin_xxx 封装函数, 永远不直接碰 GPIO 寄存器, 也不
 * #include 任何 platform_pin 内部头. 跨 MCU 移植 (STM32 -> NXP) 时这一份
 * 代码 0 改动, 只换 platform/arch/<mcu>/pin_board.c 一份.
 *
 * 见 ch15 § 15.3 子类层 + § 15.11.5 "STM32 vs NXP 换 MCU 不改应用".
 */

#include "drivers/led/led_gpio.h"
#include "platform/platform_pin.h"
#include <stddef.h>

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	platform_pin_write(self->pin, self->active_high ? PIN_HIGH : PIN_LOW);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	platform_pin_write(self->pin, self->active_high ? PIN_LOW : PIN_HIGH);
	return 0;
}

/* set_brightness 故意不填, GPIO 不支持调光, 走父类默认 no-op */
static const struct led_ops gpio_ops = {
	.on  = gpio_on,
	.off = gpio_off,
};

int led_gpio_init(struct led_gpio *me, const char *name,
                  const char *pin_name, bool active_high)
{
	int32_t pin;
	int     rc;

	if (!me || !pin_name)
		return -1;

	pin = platform_pin_get(pin_name);
	if (pin < 0)
		return -1;

	platform_pin_mode(pin, PIN_MODE_OUTPUT);
	platform_pin_write(pin, active_high ? PIN_LOW : PIN_HIGH);

	me->pin         = pin;
	me->active_high = active_high;

	rc = led_base_init(&me->base, name, &gpio_ops);
	return rc;
}
