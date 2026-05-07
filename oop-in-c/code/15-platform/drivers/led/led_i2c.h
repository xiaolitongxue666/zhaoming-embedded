/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.h - LED I2C 子类 (走 I2C bus + client 写控制寄存器).
 *
 * 演示"换硬件不改应用": 同样的 led_on / led_off, 子类底下从拉 GPIO 换成
 * 走 I2C 总线写控制寄存器. 上层应用 / 业务层一字不动.
 *
 * 这一份相比 ch15 早期教学版升级到 bus + client 二层接口, 和 industrial/
 * platform_layer/ 工业版签名对齐. 子类内嵌 struct platform_i2c_client
 * 持有 bus 句柄 + 自家 7-bit 地址; led_i2c_init 接受 bus 参数显式传入,
 * 启动期 (board_init / platform/arch/<mcu>/pin_board.c) 把 bus 注册好后
 * 才能调本子类构造.
 *
 * 不实现 set_brightness, 走父类默认 no-op.
 * 子类底下只调 platform_i2c_xxx ops 表层接口, 跨 MCU 0 改动.
 */

#ifndef DRIVERS_LED_LED_I2C_H
#define DRIVERS_LED_LED_I2C_H

#include "drivers/led/led_base.h"
#include "platform/platform_i2c.h"

struct led_i2c {
	struct led_base            base;
	struct platform_i2c_client client;   /* bus + 7-bit 从机地址 */
	uint8_t                    reg;      /* 控制寄存器地址 */
};

/* 构造函数.
 *   me         子类实例
 *   name       实例名
 *   bus        I2C bus 句柄, 由 platform_i2c_bus_get() 拿到
 *   client_addr 7-bit 从机地址 (例如 0x20)
 *   reg        控制寄存器地址 (例如 0x01)
 */
int led_i2c_init(struct led_i2c *me, const char *name,
                 struct platform_i2c_bus_device *bus,
                 uint8_t client_addr, uint8_t reg);

#endif /* DRIVERS_LED_LED_I2C_H */
