/* SPDX-License-Identifier: MIT */
/*
 * platform_i2c.h - I2C 抽象层 (bus + client 二层, 教学简化版).
 *
 * 工业 platform 层的 I2C 不是单点接口, 是一组结构体:
 *
 *   struct platform_i2c_bus_device   一条物理 I2C 总线 (= Linux i2c_adapter)
 *   struct platform_i2c_client       挂在 bus 上的一颗芯片 (= Linux i2c_client)
 *   struct platform_i2c_msg          一段读写报文 (= Linux i2c_msg)
 *
 * 一条 i2c bus 上挂多颗芯片 (温度传感器 / EEPROM / RTC / IO expander),
 * 它们共享同一组 SDA / SCL 线. 所以 i2c 比 GPIO 多一层抽象: bus 管控
 * 制器层时序 / 中断 / 锁; client 记录"我挂在哪条 bus 上, 我的 7-bit
 * 地址是多少". 上层驱动 (max31827 温度传感器 / 24Cxx EEPROM) 持有
 * 一个 client, 调 platform_i2c_transfer 走 bus.
 *
 * 教学版相比工业版做了三处简化 (但签名对齐, 看完直接能读 industrial/
 * platform_layer/platform_i2c.h 工业版):
 *   1. ops 表只保留 master_xfer, slave_xfer / control 教学版省略
 *   2. bus 结构省 mutex, 工业版有 osMutex 抢锁
 *   3. 错误码用 int 0/-1, 工业版用 platform_err_t
 *
 * 跨 MCU 移植只换 platform/arch/<mcu>/pin_board.c 一份: 那里实例化一
 * 条 bus 注册进来, 上层 drivers/led/led_i2c 字节级不动.
 */

#ifndef PLATFORM_PLATFORM_I2C_H
#define PLATFORM_PLATFORM_I2C_H

#include <stdint.h>

/* msg flags */
#define PLATFORM_I2C_WR          0x0000  /* 写方向 (默认) */
#define PLATFORM_I2C_RD          0x0001  /* 读方向 */
#define PLATFORM_I2C_NO_START    0x0002  /* 续段不发起始信号 (Repeated-Start 之后) */

/* 一段 I2C 报文. 多段 msg 拼一个 transfer, 中间用 Repeated-Start 串起来.
 * 字段含义和 Linux <linux/i2c.h> struct i2c_msg 一字不差. */
struct platform_i2c_msg {
	uint16_t addr;     /* 7-bit 从机地址 (写在低 7 位) */
	uint16_t flags;    /* PLATFORM_I2C_WR / RD / NO_START 组合 */
	uint16_t len;      /* buf 字节数 */
	uint8_t  *buf;     /* 数据缓冲区 */
};

struct platform_i2c_bus_device;

/* bus 控制器 ops 表. 由具体 MCU 后端 (platform/arch/<mcu>/pin_board.c)
 * 填写. 教学版只保留最关键的 master_xfer; 工业版还有 slave_xfer 和
 * control (设时钟频率 / 总线复位等), 见 industrial/platform_layer. */
struct platform_i2c_bus_device_ops {
	/* 在 bus 上发起一组 master 传输. 返回成功完成的 msg 段数,
	 * 0 表示总线错或 ACK 丢. */
	uint32_t (*master_xfer)(struct platform_i2c_bus_device *bus,
	                        struct platform_i2c_msg *msgs, uint32_t num);
};

/* 一条 I2C 总线 (= Linux i2c_adapter). 一颗 MCU 通常 1-4 条 bus,
 * 每条 bus 在 platform/arch/<mcu>/pin_board.c 启动期实例化一份并
 * 调 platform_i2c_bus_register 挂到 dispatcher. */
struct platform_i2c_bus_device {
	const struct platform_i2c_bus_device_ops *ops;
};

/* 一颗挂在 bus 上的 I2C 芯片 (= Linux i2c_client). 上层驱动
 * (温度传感器 / EEPROM / IO expander) 嵌入一个 client 字段, 启动
 * 期填好 bus 句柄 + 自家 7-bit 地址. */
struct platform_i2c_client {
	struct platform_i2c_bus_device *bus;   /* 我挂在哪条总线 */
	uint16_t                        client_addr; /* 我是哪片 (例如 0x48) */
};

/* ============== 公开接口 ============== */

/* 注册一条 bus. 一颗 MCU 上可挂多条, 教学版简化为单条 (后注册的覆盖
 * 前一条). 工业版按 bus_name 走 device 表查找, 见 industrial 版. */
int platform_i2c_bus_register(struct platform_i2c_bus_device *bus,
                              const struct platform_i2c_bus_device_ops *ops);

/* 取已注册的 bus. 上层驱动启动期调一次拿到 bus 句柄, 填进 client.bus. */
struct platform_i2c_bus_device *platform_i2c_bus_get(void);

/* 在指定 bus 上发起一次 transfer. msgs 多段在底层用 Repeated-Start 串起.
 * 返回成功的 msg 段数, 0 表示失败. */
uint32_t platform_i2c_transfer(struct platform_i2c_bus_device *bus,
                               struct platform_i2c_msg *msgs, uint32_t num);

#endif /* PLATFORM_PLATFORM_I2C_H */
