/* SPDX-License-Identifier: MIT */
/*
 * platform_i2c.h - I2C bus + client 二层抽象 (教学完整版).
 *
 * 跟 industrial/platform_layer/platform_i2c.h 接口字节对齐, 但去掉 osMutex /
 * platform_device parent / bus_find 这些依赖 RTOS 和设备表的部分, 保持 stm32_full
 * 教学完整版纯 C 可独立 build.
 *
 * 两层结构:
 *   struct platform_i2c_bus_device   总线 (一颗 MCU 的一路 I2C 控制器)
 *     └── ops->master_xfer           子类填: 把 msgs 数组一段段发出去
 *
 *   struct platform_i2c_client       挂在某条 bus 上的从设备 (一颗 IO expander /
 *                                     EEPROM / 传感器), 携带 7-bit 从机地址
 *
 * 上层 driver (例如 led_i2c) 拼好 platform_i2c_msg 数组, 调
 * platform_i2c_transfer(client->bus, msgs, num) 走通. 看不到底层是 STM32 I2C
 * 外设还是 mock printf, 跨芯片移植只换 i2c_board_<chip>.c.
 *
 * 工业完整版还有 slave_xfer / i2c_bus_control / master_send / master_recv,
 * 教学完整版按需省略, 保留最常用的 master_xfer 一条主路径.
 */

#ifndef PLATFORM_API_PLATFORM_I2C_H_
#define PLATFORM_API_PLATFORM_I2C_H_

#include <stdint.h>

#include "platform/platform_def.h"

/* I2C msg flags (跟 platform_layer 字节对齐) */
#define PLATFORM_I2C_WR          (0x0000)
#define PLATFORM_I2C_RD          (1u << 0)
#define PLATFORM_I2C_ADDR_10BIT  (1u << 2)  /* 10-bit chip address */

/* 一段 I2C transfer. 多段拼起来支持 "写 reg 地址 + 重启读 N 字节" 这种组合. */
struct platform_i2c_msg {
	uint16_t  addr;     /* 7-bit / 10-bit 从机地址 */
	uint16_t  flags;    /* PLATFORM_I2C_RD / WR / ADDR_10BIT */
	uint16_t  len;      /* buf 长度 */
	uint8_t  *buf;      /* 数据缓冲 */
};

struct platform_i2c_bus_device;

/* bus 子类 ops 表 (i2c_board_<chip>.c 填写) */
struct platform_i2c_bus_device_ops {
	platform_err_t (*master_xfer)(struct platform_i2c_bus_device *bus,
	                              struct platform_i2c_msg *msgs,
	                              uint32_t num);
};

/* I2C bus 一条总线. 真机有几路 I2C 外设就有几个实例.
 *
 * 跟 platform_layer 工业完整版相比: 砍掉 platform_device parent / osMutex lock /
 * priv 字段. 教学完整版不强制 RTOS, 单线程跑通即可. */
struct platform_i2c_bus_device {
	const struct platform_i2c_bus_device_ops *ops;
};

/* I2C client 挂在某条 bus 上的从设备. driver (led_i2c / EEPROM / 传感器)
 * 把 client 嵌入自己的 struct, 构造时传 bus 句柄 + 从机地址进来. */
struct platform_i2c_client {
	struct platform_i2c_bus_device *bus;
	uint16_t                        client_addr;
};

/* 注册接口 (i2c_board_<chip>.c 启动期调) */
platform_err_t platform_i2c_bus_register(struct platform_i2c_bus_device *bus,
                                         const struct platform_i2c_bus_device_ops *ops);

/* 公共 API (上层 driver 调) */
platform_err_t platform_i2c_transfer(struct platform_i2c_bus_device *bus,
                                     struct platform_i2c_msg *msgs,
                                     uint32_t num);

#endif /* PLATFORM_API_PLATFORM_I2C_H_ */
