/* SPDX-License-Identifier: MIT */
/**
 * @file  led_i2c.h
 * @brief LED I2C 子类 -- 通过 I2C 总线写控制寄存器开/关
 *
 * @details
 * 这是 ch11 第一次同时演示三种 LED 子类 (GPIO / PWM / I2C). 三种 LED
 * 共用 struct led_base + struct led_ops, 父类一行 led_on(base) dispatch
 * 出三种行为. 这就是多态完整图景.
 *
 * I2C LED 真实硬件场景: PCA9554 / TCA6408 这类 IO expander 通过 I2C
 * 拓展更多通道, 主控只发一次写寄存器命令就开/关一颗 LED. 教学版
 * 在 PC 上用 printf 模拟一次"I2C 写寄存器"动作.
 */

#ifndef LED_I2C_H
#define LED_I2C_H

#include "led_base.h"

struct led_i2c {
	struct led_base base;
	uint8_t         dev_addr;   /* I2C 7 位从设备地址 */
	uint8_t         reg;        /* 控制寄存器地址 */
};

int led_i2c_init(struct led_i2c *me, const char *name,
                 uint8_t dev_addr, uint8_t reg);

extern const struct led_ops led_ops_i2c;

#endif /* LED_I2C_H */
