/* SPDX-License-Identifier: MIT */
/*
 * platform_i2c.c - I2C bus + client dispatcher (教学简化版).
 *
 * 教学版只挂一条 bus (单 dispatcher). 工业版按 bus_name 走全局
 * platform_device 表, 一颗 MCU 上可同时跑 4 条 bus (i2c1 / i2c2 ...).
 * 见 industrial/platform_layer/platform_i2c.c.
 *
 * 锁: 工业版 master_xfer 全程持 osMutex 兜底多任务争用. 教学版省略,
 * 让读者把注意力放在二层抽象本身.
 */

#include "platform/platform_i2c.h"
#include <stddef.h>

/* 教学版单 bus 槽. 工业版用 platform_device 全局表 + bus_name 查找. */
static struct platform_i2c_bus_device *_g_bus = NULL;

int platform_i2c_bus_register(struct platform_i2c_bus_device *bus,
                              const struct platform_i2c_bus_device_ops *ops)
{
	if (!bus || !ops)
		return -1;
	bus->ops = ops;
	_g_bus   = bus;
	return 0;
}

struct platform_i2c_bus_device *platform_i2c_bus_get(void)
{
	return _g_bus;
}

uint32_t platform_i2c_transfer(struct platform_i2c_bus_device *bus,
                               struct platform_i2c_msg *msgs, uint32_t num)
{
	if (!bus || !bus->ops || !bus->ops->master_xfer || !msgs || num == 0)
		return 0;
	return bus->ops->master_xfer(bus, msgs, num);
}
