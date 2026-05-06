/* SPDX-License-Identifier: MIT */
/*
 * leds_gpio.c - 内核里的 leds-gpio 驱动山寨版
 *
 * 真实内核版定义在 drivers/leds/leds-gpio.c。它就调一行 gpiod_set_value，
 * 不关心底下是哪家 SoC 的 GPIO 控制器。
 *
 * 这就是 Linux 内核驱动作者的世界：通过 ops 表 + 抽象接口，写一份代码
 * 服务所有芯片。如果没有 gpiolib 这一层抽象，你要写 N 份 leds-gpio
 * （每家 SoC 一份），N × M 份代码（M 个上层设备驱动 × N 家 SoC）；
 * 加了 gpiolib，N + M 份就够。这就是 ch16 § 16.4 "乘法变加法"的本质。
 */

#include "gpio_chip.h"
#include <stdio.h>

void led_gpio_brightness_set(struct gpio_desc *desc, int value)
{
	/*
	 * 这一行内部走 desc->gc->set，多态 dispatch。
	 * vendorA 的 desc 走 vendor_a_set，vendorB 的 desc 走 vendor_b_set。
	 * 这个驱动不知道也不需要知道——这就是分层抽象给上层驱动作者
	 * 的最大礼物：写一份代码服务所有芯片。
	 */
	gpiod_set_value(desc, value);
}
