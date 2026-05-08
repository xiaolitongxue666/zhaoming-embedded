/* SPDX-License-Identifier: MIT */
/*
 * platform/arch/nxp/i2c_board.c - NXP i.MX RT1170 端 I2C bus 后端实现.
 *
 * "认识 NXP LPI2C 外设"的唯一一份文件. 上层 drivers/led/led_i2c 拼好
 * platform_i2c_msg 数组, 走 platform_i2c_transfer ops 表层接口下来,
 * 落到这里之后才碰 LPI2C_Type * 加 LPI2C_MasterTransferBlocking.
 *
 * 跨 MCU (STM32 -> NXP) 的"换 6 份之一": 本文 + 同目录 pin_board.c +
 * pwm_board.c 三份是这家 MCU 的 platform 实现层.
 *
 * 唯一不同 (对照 stm32/i2c_board.c):
 *   I2C_HandleTypeDef *           ->  LPI2C_Type *
 *   HAL_I2C_Master_Transmit/Recv  ->  LPI2C_MasterTransferBlocking
 *   addr8 = addr << 1 (HAL 内部含 r/w 位) -> 直接传 7-bit slaveAddress
 *
 * 教学版只支持一条 i2c bus + 阻塞 master_xfer. 工业版多 bus + DMA + osMutex,
 * 见 industrial/platform_layer/.
 *
 * 工程假设: MCUXpresso IDE 已经初始化 LPI2C1 (Standard Mode 100 kHz),
 * fsl_lpi2c 头是 SDK builtin.
 */

#include "platform/platform_i2c.h"
#include "fsl_lpi2c.h"

/* 多段 msg 通过 LPI2C SDK 一段一段下发. 教学版串行翻译, 工业版按
 * NXP SDK 的 lpi2c_master_transfer_t 一次性 multi-msg 走 DMA. */
static uint32_t _nxp_i2c_master_xfer(struct platform_i2c_bus_device *bus,
                                     struct platform_i2c_msg *msgs,
                                     uint32_t num)
{
	lpi2c_master_transfer_t xfer = {0};
	status_t                st = kStatus_Success;
	uint32_t                i;

	(void)bus;

	for (i = 0; i < num && st == kStatus_Success; i++) {
		xfer.slaveAddress   = (uint8_t)msgs[i].addr;
		xfer.direction      = (msgs[i].flags & PLATFORM_I2C_RD)
		                      ? kLPI2C_Read : kLPI2C_Write;
		xfer.subaddressSize = 0;
		xfer.data           = msgs[i].buf;
		xfer.dataSize       = msgs[i].len;
		st = LPI2C_MasterTransferBlocking(LPI2C1, &xfer);
	}
	return (st == kStatus_Success) ? num : 0;
}

static const struct platform_i2c_bus_device_ops _nxp_i2c_ops = {
	.master_xfer = _nxp_i2c_master_xfer,
};

/* 一条 i2c bus 实例. 教学版单 bus, 工业版按 bus_name 走 device 表挂多条. */
static struct platform_i2c_bus_device _nxp_i2c_bus;

/* 启动期由 platform_init 调一次. 调用顺序: pin -> pwm -> i2c. */
void platform_hw_i2c_init(void)
{
	(void)platform_i2c_bus_register(&_nxp_i2c_bus, &_nxp_i2c_ops);
}
