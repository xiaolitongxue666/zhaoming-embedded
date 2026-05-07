/* SPDX-License-Identifier: MIT */
/*
 * i2c_board_pc.c - PC mock board-level I2C bus driver.
 *
 * 用 printf 假装一颗 MCU 的一路 I2C 控制器, 演示 led_i2c 子类发出的 master
 * transfer. 启动期通过 INIT_BOARD_EXPORT 自动构造一个 platform_i2c_bus_device
 * 实例 + 注册 ops.
 *
 * 真机版换成 platform_hw_i2c_<chip>.c, master_xfer 里调具体 HAL 的 I2C 主收发
 * (HAL_I2C_Master_Transmit 之类), bus 实例可以一颗 MCU 装多条 (i2c1 / i2c2 /
 * i2c3), 各自挂自己的 ops.
 */

#include <stddef.h>
#include <stdio.h>

#include "platform/platform_i2c.h"
#include "platform/platform_module_export.h"

/* 提供给 led_cfg.c 用的 bus 句柄 (extern). 真机版会有 i2c1_bus / i2c2_bus
 * 多条; mock 只演示一条. */
struct platform_i2c_bus_device pc_i2c_bus0;

static platform_err_t _pc_i2c_master_xfer(struct platform_i2c_bus_device *bus,
                                          struct platform_i2c_msg *msgs,
                                          uint32_t num)
{
	uint32_t i;
	uint16_t j;

	(void)bus;

	if (NULL == msgs) {
		return PLATFORM_EINVAL;
	}

	for (i = 0; i < num; i++) {
		const struct platform_i2c_msg *m = &msgs[i];
		const char *dir = (m->flags & PLATFORM_I2C_RD) ? "R" : "W";

		printf("    [I2C] addr=0x%02x %s len=%u data=",
		       (unsigned)m->addr, dir, (unsigned)m->len);
		if ((m->flags & PLATFORM_I2C_RD) || (NULL == m->buf)) {
			printf("(none)");
		} else {
			for (j = 0; j < m->len; j++) {
				printf("%02x ", (unsigned)m->buf[j]);
			}
		}
		printf("\n");
	}

	return PLATFORM_EOK;
}

static const struct platform_i2c_bus_device_ops _pc_i2c_bus_ops = {
	.master_xfer = _pc_i2c_master_xfer,
};

static void _i2c_board_init(void)
{
	(void)platform_i2c_bus_register(&pc_i2c_bus0, &_pc_i2c_bus_ops);
}
INIT_BOARD_EXPORT(_i2c_board_init);
