/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.h - LED I2C 子类 (走 I2C bus + client 二层抽象).
 *
 * 这一份演示"换硬件不改应用": 同样的 led_on / led_off, 子类底下从拉 GPIO
 * 换成走 I2C 总线写控制寄存器. 上层应用 / 业务层一字不动.
 *
 * 用 platform_i2c_client 嵌入字段携带 (bus, client_addr), 真发命令时调
 * platform_i2c_transfer(client.bus, msgs, 1). 跟 EEPROM / 传感器 / IO expander
 * 这些 client 走的是同一套接口.
 *
 * 不实现 set_brightness, 走父类默认 no-op.
 */

#ifndef __LED_I2C_H
#define __LED_I2C_H

#include <stdint.h>

#include "led_base.h"
#include "platform_def.h"
#include "platform_i2c.h"

struct led_i2c {
	struct led_base             base;
	struct platform_i2c_client  client;    /* 嵌入式 client (bus + 从机地址) */
	uint8_t                     reg;       /* 控制寄存器地址 */
	uint8_t                     val_on;    /* 写入这个值表示 ON */
	uint8_t                     val_off;   /* 写入这个值表示 OFF */
};

/* 构造函数.
 *   me           子类实例
 *   name         实例名
 *   bus          挂的 I2C 总线 (上一层注册好)
 *   client_addr  I2C 7-bit 从设备地址
 *   reg          控制寄存器地址
 */
platform_err_t led_i2c_init(struct led_i2c *me, const char *name,
                            struct platform_i2c_bus_device *bus,
                            uint16_t client_addr, uint8_t reg);

#endif /* __LED_I2C_H */
