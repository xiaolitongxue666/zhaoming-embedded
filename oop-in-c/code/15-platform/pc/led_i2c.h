/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.h - 子类三: I2C 扩展芯片 LED (ch15 完整版, bus + client 二层)
 *
 * 走 I2C 总线给某个寄存器写 0/1 控制一颗 LED 亮灭. 硬件资源不再是
 * 简单的 (bus 编号 + 7-bit 地址), 而是嵌一个 struct platform_i2c_client:
 *   client.bus          指向哪条 i2c 总线 (struct platform_i2c_bus_device *)
 *   client.client_addr  7-bit 从机地址 (例如 0x3C)
 *
 * 二层抽象把 "总线控制器" 和 "挂在总线上的某颗芯片" 分开 -- bus 由 board
 * 启动期实例化 + 注册, client 由具体外设构造期填好. 见 ch15 § 15.17.2.
 *
 * 这种结构在工业控制板里很常见 (主控 GPIO 不够用, 挂一颗 PCA9555 之类
 * 的 I/O 扩展芯片). 应用层完全不知道, 它只调 led_on(handle).
 *
 * 这一份子类头只装 struct led_i2c + 构造函数声明. 应用层永远不该
 * #include 它 -- 应用层只 #include "leds.h" 拿 base 句柄.
 */

#ifndef LED_I2C_H
#define LED_I2C_H

#include "led_base.h"
#include "platform/platform_i2c.h"

struct led_i2c {
	struct led_base            base;     /* 父类, 第 0 字段 */
	struct platform_i2c_client client;   /* bus + 7-bit 从机地址 */
	uint8_t                    reg;      /* 控制寄存器地址 */
};

/* 构造函数.
 *   me           子类实例
 *   name         实例名 (用于打印)
 *   bus          I2C bus 句柄, 由 platform_i2c_bus_get() 拿到
 *   client_addr  7-bit 从机地址 (例如 0x3C)
 *   reg          控制寄存器地址 (例如 0x00)
 */
int led_i2c_init(struct led_i2c *me, const char *name,
                 struct platform_i2c_bus_device *bus,
                 uint8_t client_addr, uint8_t reg);

#endif /* LED_I2C_H */
