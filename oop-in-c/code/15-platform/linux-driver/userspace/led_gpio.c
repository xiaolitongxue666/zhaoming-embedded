/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.c - 子类一: GPIO LED 实现 (ch15 linux-driver/userspace 版).
 *
 * 直接调 libgpiod 1.x. 没有 platform_gpio_xxx 中间层 -- 内核已经做完.
 * libgpiod 接口本身就是 "申请 line + set value" 这种 OOP 风格, 套一层
 * 自家的 platform_gpio_write -> gpiod_line_set_value 反而多一层 indirection.
 *
 * 这是 ch15 § 15.11 / § 15.15 / 附录 C 的代码兑现点: 应用层 OOP 抽象保留
 * (struct led_base + ops 表), 工程师自抽的 platform 抽象层删除.
 *
 * libgpiod 1.x API (Debian 12 / Ubuntu 22.04 默认装的版本). 2.x API 改成
 * request builder 风格, 移植到 2.x 改 init 里两行调用即可.
 */

#include "led_gpio.h"
#include <gpiod.h>
#include <stddef.h>
#include <stdio.h>

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	int rc;

	rc = gpiod_line_set_value(self->line, self->active_high ? 1 : 0);
	if (rc < 0)
		return -1;
	me->is_on = true;
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	int rc;

	rc = gpiod_line_set_value(self->line, self->active_high ? 0 : 1);
	if (rc < 0)
		return -1;
	me->is_on = false;
	return 0;
}

/* set_brightness 故意不填: GPIO 不支持调光, 走父类默认行为. */
static const struct led_ops gpio_ops = {
	.on  = gpio_on,
	.off = gpio_off,
};

int led_gpio_init(struct led_gpio *me, const char *name,
                  struct gpiod_chip *chip, unsigned int line_offset,
                  bool active_high)
{
	struct gpiod_line *line;
	int rc;
	int initial_value;

	if (!me || !name || !chip)
		return -1;

	line = gpiod_chip_get_line(chip, line_offset);
	if (!line) {
		fprintf(stderr, "[led_gpio:%s] get_line(%u) failed\n",
		        name, line_offset);
		return -1;
	}

	/* 上电先关灯 */
	initial_value = active_high ? 0 : 1;
	rc = gpiod_line_request_output(line, "led", initial_value);
	if (rc < 0) {
		fprintf(stderr, "[led_gpio:%s] request_output failed\n", name);
		return -1;
	}

	me->line        = line;
	me->active_high = active_high;
	return led_base_init(&me->base, name, &gpio_ops);
}

void led_gpio_deinit(struct led_gpio *me)
{
	if (!me || !me->line)
		return;
	gpiod_line_release(me->line);
	me->line = NULL;
}
