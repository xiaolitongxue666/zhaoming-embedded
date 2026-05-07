/* SPDX-License-Identifier: MIT */
/**
 * @file  led_i2c.h
 * @brief LED I2C 扩展芯片子类 (ch12 版)
 *
 * @details
 * 走 I2C 总线给某个寄存器写 0/1 控制一颗 LED 亮灭. 硬件资源: 总线
 * 编号 + 7-bit 设备地址.
 *
 * 这种结构在工业控制板里很常见 (主控 GPIO 不够用, 挂一颗 PCA9555
 * 之类的 I/O 扩展芯片). 应用层完全不知道, 它只调 led_on(handle).
 */

#ifndef LED_I2C_H
#define LED_I2C_H

#include "led.h"

struct led_i2c {
	struct led_base base;       /* 父类, 第 0 字段 */
	uint8_t         bus;
	uint8_t         addr;
};

int led_i2c_init(struct led_i2c *me, const char *name,
                 uint8_t bus, uint8_t addr);

extern const struct led_ops led_ops_i2c;

#endif /* LED_I2C_H */
