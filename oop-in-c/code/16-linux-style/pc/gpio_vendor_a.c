/* SPDX-License-Identifier: MIT */
/*
 * gpio_vendor_a.c - 厂商 A 的 gpio_chip 驱动
 *
 * 假装这是某家 SoC 的 GPIO 控制器实现。把 chip->set 指向自己的
 * 寄存器操作（这里写 DR_REG 这种"数据寄存器一次写整字"的风格）。
 * 在 PC 上用 printf 模拟。
 *
 * 这一份对应"按功能定义接口、按 chip 提供实现"。上层 led_gpio_brightness_set
 * 不需要知道这是厂商 A 的芯片，它只调 gpiod_set_value，gpiolib 自己
 * dispatch 到 vendor_a_set。
 *
 * 真实 Linux 内核里的等价物：drivers/gpio/gpio-mxc.c、gpio-rockchip.c
 * 等等。每家芯片厂提供一份这样的文件。见 ch16 § 16.5。
 */

#include "gpio_chip.h"
#include <stdio.h>

static int vendor_a_request(struct gpio_chip *gc, unsigned int offset)
{
	printf("    [vendorA] request offset=%u (write reg PORT_EN)\n",
	       offset);
	(void)gc;
	return 0;
}

static void vendor_a_free(struct gpio_chip *gc, unsigned int offset)
{
	printf("    [vendorA] free offset=%u\n", offset);
	(void)gc;
}

static int vendor_a_direction_output(struct gpio_chip *gc,
				     unsigned int offset, int value)
{
	printf("    [vendorA] direction_output offset=%u (write reg DIR)\n",
	       offset);
	(void)gc;
	(void)value;
	return 0;
}

static int vendor_a_get(struct gpio_chip *gc, unsigned int offset)
{
	(void)gc;
	(void)offset;
	return 0;
}

static void vendor_a_set(struct gpio_chip *gc, unsigned int offset, int value)
{
	printf("    [vendorA] set offset=%u value=%d (DR_REG <- 0x%08X)\n",
	       offset, value, value ? (1u << offset) : 0);
	(void)gc;
}

static struct gpio_chip vendor_a_chip = {
	.label            = "vendorA-gpio",
	.base             = 0,
	.ngpio            = 32,
	.request          = vendor_a_request,
	.free             = vendor_a_free,
	.direction_output = vendor_a_direction_output,
	.get              = vendor_a_get,
	.set              = vendor_a_set,
};

void vendor_a_probe(void)
{
	gpiochip_add(&vendor_a_chip);
}
