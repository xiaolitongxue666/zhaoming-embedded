/* SPDX-License-Identifier: MIT */
/*
 * platform_i2c_pc.c - PC 端 platform_i2c bus 实现 + 注册 (ch15 教学).
 *
 * 角色和 platform/arch/stm32/pin_board.c 里 _stm32_i2c_master_xfer 同款,
 * 区别只在: STM32 端把 platform_i2c_msg 翻译成 HAL_I2C_Master_Transmit /
 * Receive 真实 I2C 控制器寄存器, 这里翻译成 stdout printf. 同一份
 * drivers/ 子类源码 (led_i2c.c) 不动, 跨平台跑同一个二层 dispatch.
 *
 * 落地形态:
 *   pc_i2c_bus       static struct platform_i2c_bus_device 一个槽
 *   pc_i2c_bus_ops   static const struct platform_i2c_bus_device_ops 一张表
 *   platform_pc_i2c_init  启动期由 platform_init 调一次, 把 ops 注册进
 *                         platform/platform_i2c.c 的 dispatcher
 *
 * 装配: platform_init 先调 platform_pc_i2c_init 注册好 bus, 之后
 * led_board_init 调 platform_i2c_bus_get 拿到 bus 句柄, 喂给
 * led_i2c_init 装进 client.bus.
 *
 * 角色 = 真机 platform/arch/<mcu>/i2c_board.c; 注册进
 * ../platform/platform_i2c.c dispatcher. 详见 pc/README.md.
 *
 * 输出格式:
 *   [I2C] addr=0x3C W len=2 data=00 01
 *   [I2C] addr=0x3C W len=2 data=00 00
 * 让读者第一眼跑 demo 就看到二层兑现 (msg.addr 是 client 的 7-bit 地址,
 * msg.buf 第 0 字节是 reg 寄存器地址, 第 1 字节是写入值).
 */

#include "platform/platform_i2c.h"
#include <stdio.h>

static uint32_t _pc_i2c_master_xfer(struct platform_i2c_bus_device *bus,
                                    struct platform_i2c_msg *msgs,
                                    uint32_t num)
{
	uint32_t i;
	uint16_t j;

	(void)bus;

	for (i = 0; i < num; i++) {
		char dir = (msgs[i].flags & PLATFORM_I2C_RD) ? 'R' : 'W';
		printf("  [I2C] addr=0x%02X %c len=%u",
		       (unsigned)msgs[i].addr, dir, (unsigned)msgs[i].len);
		if (!(msgs[i].flags & PLATFORM_I2C_RD) && msgs[i].len > 0) {
			printf(" data=");
			for (j = 0; j < msgs[i].len; j++)
				printf("%02X ", msgs[i].buf[j]);
		}
		printf("\n");
	}
	return num;
}

static const struct platform_i2c_bus_device_ops pc_i2c_bus_ops = {
	.master_xfer = _pc_i2c_master_xfer,
};

static struct platform_i2c_bus_device pc_i2c_bus;

/* 启动期注册 PC i2c bus. platform_init 调一次. */
void platform_pc_i2c_init(void)
{
	(void)platform_i2c_bus_register(&pc_i2c_bus, &pc_i2c_bus_ops);
}
