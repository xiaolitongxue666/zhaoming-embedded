/* SPDX-License-Identifier: MIT */
/*
 * platform/arch/stm32/i2c_board.c - STM32F4 端 I2C bus 后端实现.
 *
 * "认识 STM32 I2C 外设"的唯一一份文件. 上层 drivers/led/led_i2c 拼好
 * platform_i2c_msg 数组, 走 platform_i2c_transfer ops 表层接口下来,
 * 落到这里之后才碰 I2C_HandleTypeDef * 加 HAL_I2C_Master_Transmit /
 * HAL_I2C_Master_Receive.
 *
 * 跨 MCU (STM32 -> NXP) 的"换 6 份之一": 同目录 pin_board.c + pwm_board.c
 * + 本文三份是这家 MCU 的 platform 实现层. 上层 drivers/led/led_i2c +
 * platform/platform_i2c.* 字节级不变.
 *
 * 教学版只支持一条 i2c bus + 阻塞 master_xfer (每段 100 ms 超时).
 * 工业版多 bus + 中断 / DMA + osMutex, 见 industrial/platform_layer/.
 *
 * 工程假设: STM32CubeMX 已配好 I2C1 (Standard Mode 100 kHz), 生成
 * hi2c1 全局句柄. 真实工程改 #include 头 + 把 hi2c1 换成实际句柄.
 */

#include "platform/platform_i2c.h"
#include "stm32f4xx_hal.h"

/* CubeMX 生成的外设句柄. 这里仅声明, 真机工程在 main.c / *_msp.c 给出定义. */
extern I2C_HandleTypeDef hi2c1;

/* 把 platform_i2c_msg 翻译成 STM32 HAL 调用. 教学版串行翻译每段 msg
 * (单段 read / 单段 write / 两段 write+read 模拟 Repeated-Start).
 * 工业版走更通用的 multi-msg 状态机 + DMA, 见 industrial 版. */
static uint32_t _stm32_i2c_master_xfer(struct platform_i2c_bus_device *bus,
                                       struct platform_i2c_msg *msgs,
                                       uint32_t num)
{
	HAL_StatusTypeDef st = HAL_OK;
	uint32_t          i;

	(void)bus;

	for (i = 0; i < num && st == HAL_OK; i++) {
		uint16_t addr8 = (uint16_t)(msgs[i].addr << 1);

		if (msgs[i].flags & PLATFORM_I2C_RD) {
			st = HAL_I2C_Master_Receive(&hi2c1, addr8,
			                            msgs[i].buf, msgs[i].len, 100);
		} else {
			st = HAL_I2C_Master_Transmit(&hi2c1, addr8,
			                             msgs[i].buf, msgs[i].len, 100);
		}
	}
	return (st == HAL_OK) ? num : 0;
}

static const struct platform_i2c_bus_device_ops _stm32_i2c_ops = {
	.master_xfer = _stm32_i2c_master_xfer,
};

/* 一条 i2c bus 实例. 教学版单 bus, 工业版按 bus_name 走 device 表挂多条. */
static struct platform_i2c_bus_device _stm32_i2c_bus;

/* 启动期由 board_init 调一次. 调用顺序: pin -> pwm -> i2c. */
void platform_hw_i2c_init(void)
{
	(void)platform_i2c_bus_register(&_stm32_i2c_bus, &_stm32_i2c_ops);
}
