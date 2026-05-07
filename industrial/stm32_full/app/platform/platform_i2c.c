/* SPDX-License-Identifier: MIT */
/*
 * platform_i2c.c - I2C bus framework dispatcher.
 *
 * 教学完整版: 注册一条 bus 时只是把 ops 挂到 bus 实例上, 不维护全局 bus 表
 * (工业完整版才需要 platform_i2c_bus_device_find 按名查). transfer 直接走
 * bus->ops->master_xfer, 单线程不上锁; 真要并发再换到 platform_layer 的
 * osMutex 版本.
 */

#include <stddef.h>

#include "platform/platform_i2c.h"
#include "platform/platform_assert.h"

platform_err_t platform_i2c_bus_register(struct platform_i2c_bus_device *bus,
                                         const struct platform_i2c_bus_device_ops *ops)
{
	platform_err_t ret = PLATFORM_EINVAL;

	if ((NULL == bus) || (NULL == ops)) {
		goto exit;
	}

	bus->ops = ops;
	ret = PLATFORM_EOK;

exit:
	return ret;
}

platform_err_t platform_i2c_transfer(struct platform_i2c_bus_device *bus,
                                     struct platform_i2c_msg *msgs,
                                     uint32_t num)
{
	platform_err_t ret = PLATFORM_ENOSYS;

	platform_assert(bus != NULL);
	platform_assert(bus->ops != NULL);
	platform_assert(msgs != NULL);

	if (NULL != bus->ops->master_xfer) {
		ret = bus->ops->master_xfer(bus, msgs, num);
	}

	return ret;
}
