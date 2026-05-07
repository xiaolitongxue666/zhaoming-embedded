/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.c - LED GPIO 子类实现 (基于 libgpiod 直接调用).
 *
 * 没有任何 platform 抽象层. 底下就是 libgpiod, libgpiod 背后就是 Linux
 * 内核的 gpio chardev / gpiolib (内核完整 driver model). 应用层再套一层
 * platform 就是反工程.
 *
 * libgpiod 1.x API (Debian 12 / Ubuntu 22.04 默认装的版本). 2.x API 改成了
 * request builder 风格, 移植到 2.x 改 init 里两行调用即可.
 */

#include <gpiod.h>
#include <stddef.h>
#include <stdio.h>

#include "drivers/led/led_gpio.h"

static platform_err_t _led_gpio_on(struct led_base *me);
static platform_err_t _led_gpio_off(struct led_base *me);

/* static const ops 表给所有 led_gpio 实例共享, 放只读段. */
static const struct led_ops led_gpio_ops = {
	.on             = _led_gpio_on,
	.off            = _led_gpio_off,
	.set_brightness = NULL,    /* GPIO LED 不支持调亮度, 走父类默认 no-op */
};

platform_err_t led_gpio_init(struct led_gpio *me, const char *name,
                             struct gpiod_chip *chip,
                             unsigned int line_offset, bool active_high)
{
	struct gpiod_line *line;
	int                rc;
	platform_err_t     ret;
	int                initial_value;

	if ((NULL == me) || (NULL == name) || (NULL == chip)) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	line = gpiod_chip_get_line(chip, line_offset);
	if (NULL == line) {
		fprintf(stderr, "[led_gpio:%s] get_line(%u) failed\n",
		        name, line_offset);
		ret = PLATFORM_EIO;
		goto exit;
	}

	/* 默认让 LED 处于"灭"状态 */
	initial_value = active_high ? 0 : 1;
	rc = gpiod_line_request_output(line, "led", initial_value);
	if (rc < 0) {
		fprintf(stderr, "[led_gpio:%s] request_output failed\n", name);
		ret = PLATFORM_EIO;
		goto exit;
	}

	me->line        = line;
	me->active_high = active_high;

	ret = led_base_init(&me->base, name, &led_gpio_ops);

exit:
	return ret;
}

void led_gpio_deinit(struct led_gpio *me)
{
	if ((NULL == me) || (NULL == me->line)) {
		return;
	}
	gpiod_line_release(me->line);
	me->line = NULL;
}

static platform_err_t _led_gpio_on(struct led_base *me)
{
	struct led_gpio *gpio = (struct led_gpio *)me;
	int              rc;

	rc = gpiod_line_set_value(gpio->line, gpio->active_high ? 1 : 0);
	if (rc < 0) {
		return PLATFORM_EIO;
	}
	return PLATFORM_EOK;
}

static platform_err_t _led_gpio_off(struct led_base *me)
{
	struct led_gpio *gpio = (struct led_gpio *)me;
	int              rc;

	rc = gpiod_line_set_value(gpio->line, gpio->active_high ? 0 : 1);
	if (rc < 0) {
		return PLATFORM_EIO;
	}
	return PLATFORM_EOK;
}
