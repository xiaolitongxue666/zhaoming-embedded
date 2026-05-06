/* SPDX-License-Identifier: MIT */
/*
 * gpio_vendor_b.c - 厂商 B 的 gpio_chip 驱动
 *
 * 同样的接口（gpio_chip 5 个函数字段），不同的内部实现：vendor_a 用
 * "数据寄存器一次写整字"，vendor_b 用"SET 寄存器 / CLR 寄存器"分高
 * 16 位 / 低 16 位（类似 STM32 BSRR 的写法，原子单 store）。
 *
 * 这一份是为了演示同一份 gpiod_set_value 在不同芯片下走到不同实现，
 * 上层驱动 (leds_gpio.c) 完全不知道这种差异。这就是 ch16 § 16.4
 * "1 × N → N + M 乘法变加法"的关键。见 ch16 § 16.5 真实内核。
 */

#include "gpio_chip.h"
#include <stdio.h>

static int vendor_b_request(struct gpio_chip *gc, unsigned int offset)
{
	printf("    [vendorB] request offset=%u (clear reg LOCK)\n", offset);
	(void)gc;
	return 0;
}

static void vendor_b_free(struct gpio_chip *gc, unsigned int offset)
{
	printf("    [vendorB] free offset=%u\n", offset);
	(void)gc;
}

static int vendor_b_direction_output(struct gpio_chip *gc,
				     unsigned int offset, int value)
{
	printf("    [vendorB] direction_output offset=%u (set reg MODE)\n",
	       offset);
	(void)gc;
	(void)value;
	return 0;
}

static int vendor_b_get(struct gpio_chip *gc, unsigned int offset)
{
	(void)gc;
	(void)offset;
	return 0;
}

static void vendor_b_set(struct gpio_chip *gc, unsigned int offset, int value)
{
	/* 厂商 B 用 SET / CLR 两个寄存器（类似 STM32 BSRR） */
	uint32_t reg = value ? (1u << offset) : (1u << (offset + 16));
	printf("    [vendorB] set offset=%u value=%d (BSRR <- 0x%08X)\n",
	       offset, value, reg);
	(void)gc;
}

static struct gpio_chip vendor_b_chip = {
	.label            = "vendorB-gpio",
	.base             = 32,
	.ngpio            = 16,
	.request          = vendor_b_request,
	.free             = vendor_b_free,
	.direction_output = vendor_b_direction_output,
	.get              = vendor_b_get,
	.set              = vendor_b_set,
};

void vendor_b_probe(void)
{
	gpiochip_add(&vendor_b_chip);
}
