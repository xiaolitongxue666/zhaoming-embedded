/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.h - I2C LED 子类 (ch13 版)
 *
 * I2C 子类 base 在第 0 字段, container_of 偏移为 0 编译器会把减法优
 * 化掉. 这跟 PWM 子类一样. 三种子类全部用 container_of 是为了让
 * GPIO 那条 base 偏移 4 的教学变形不显得特殊.
 */

#ifndef LED_I2C_H
#define LED_I2C_H

#include "led.h"

struct led_i2c {
	struct led_base base;
	uint8_t         bus;
	uint8_t         addr;
};

int led_i2c_init(struct led_i2c *me, const char *name,
		 uint8_t bus, uint8_t addr);

extern const struct led_ops led_ops_i2c;

#endif /* LED_I2C_H */
