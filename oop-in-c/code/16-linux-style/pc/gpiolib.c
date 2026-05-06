/* SPDX-License-Identifier: MIT */
/*
 * gpiolib.c - "山寨" 内核 gpiolib 的最小内核态
 *
 * 注册一组 gpio_chip，通过 desc 反向找到 chip，调 chip->set / chip->get。
 * 真实内核版见 drivers/gpio/gpiolib.c 第 3245 行 gpiod_set_value。
 *
 * 内核里那段关键调用是这样：
 *   gpiod_set_value -> gpiod_set_value_nocheck -> gpiod_set_raw_value_commit
 *   -> gc->set(gc, gpio_chip_hwgpio(desc), value);
 *
 * 最后一行 gc->set 就是你 ch11 学的多态 dispatch。同一行 gpiod_set_value
 * 调用，根据 desc->gc 当前指向哪家厂商的 gpio_chip，落到不同实现。
 * 这就是 Linux 内核为什么能"一份 leds-gpio 驱动跑遍 N 家 SoC"。
 *
 * 见 ch16 § 16.5 真实代码 + ch11 多态 dispatch。
 */

#include "gpio_chip.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CHIPS	8

static struct gpio_chip *s_chips[MAX_CHIPS];
static int s_num_chips;

int gpiochip_add(struct gpio_chip *gc)
{
	if (s_num_chips >= MAX_CHIPS)
		return -1;
	s_chips[s_num_chips++] = gc;
	printf("[gpiolib] chip '%s' registered (base=%u, ngpio=%u)\n",
	       gc->label, gc->base, gc->ngpio);
	return 0;
}

struct gpio_desc *gpio_get_desc(const char *chip_label, unsigned int offset)
{
	for (int i = 0; i < s_num_chips; i++) {
		if (strcmp(s_chips[i]->label, chip_label) == 0) {
			struct gpio_desc *d = malloc(sizeof(*d));
			d->gc     = s_chips[i];
			d->offset = offset;
			return d;
		}
	}
	return NULL;
}

void gpiod_set_value(struct gpio_desc *desc, int value)
{
	if (!desc || !desc->gc)
		return;

	/* 这一行是 Linux 内核 drivers/gpio/gpiolib.c L3057 的山寨版：
	 *   gc->set(gc, gpio_chip_hwgpio(desc), value);
	 * 把 desc->offset 当 hwgpio 直接传过去。
	 */
	desc->gc->set(desc->gc, desc->offset, value);
}

int gpiod_get_value(struct gpio_desc *desc)
{
	if (!desc || !desc->gc)
		return -1;
	return desc->gc->get(desc->gc, desc->offset);
}
