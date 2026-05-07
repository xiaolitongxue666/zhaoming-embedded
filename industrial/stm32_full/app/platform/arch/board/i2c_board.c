/* SPDX-License-Identifier: MIT */
/*
 * i2c_board.c - STM32 board-level I2C bus driver implementation (HAL based).
 *
 * STM32 真机 i2c bus 子类: 一颗 MCU 一路 I2C 控制器 = 一个
 * platform_i2c_bus_device 实例 + 一份 ops 表. 这一份只挂一条 i2c1, 多条
 * 总线复制这个文件改 hi2cx 句柄 + 实例名即可.
 *
 * 应用层 led_cfg.c 装配 led_i2c client 时 extern 这里导出的
 * stm32_i2c1_bus 指针, 跟 mock/i2c_board_pc.c 的 pc_i2c_bus0 接口位置对称.
 *
 * 教学完整版砍掉 osMutex 锁 / DMA / 中断回调, 主路径 master_xfer 一条. 工业
 * 版要并发 / DMA 再换 industrial/platform_layer 的 osMutex 版本.
 */

#include <stddef.h>

#include "platform/platform_i2c.h"
#include "platform/platform_module_export.h"
#include "stm32f4xx_hal.h"

/* CubeMX 生成的 I2C 句柄, 真机 main 侧 MX_I2C1_Init 之后填进去, 这里 extern. */
extern I2C_HandleTypeDef hi2c1;

/* I2C transfer 阻塞超时 (ms). 教学版固定 1s, 工业版可改成 platform_i2c_msg
 * 字段或全局 config 控制. */
#define STM32_I2C_TIMEOUT_MS     1000u

static platform_err_t _stm32_i2c_master_xfer(struct platform_i2c_bus_device *bus,
                                             struct platform_i2c_msg *msgs,
                                             uint32_t num)
{
	platform_err_t     ret = PLATFORM_EOK;
	HAL_StatusTypeDef  st;
	uint32_t           i;
	uint16_t           hal_addr;

	(void)bus;

	if (NULL == msgs) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	for (i = 0; i < num; i++) {
		/* 7-bit 地址 HAL 要左移一位填 R/W bit (HAL 内部按位写). 10-bit
		 * 模式教学版未支持, 真要用换 HAL_I2C_Mem_Read/Write 或 ADDR_10BIT 分支. */
		hal_addr = (uint16_t)(msgs[i].addr << 1);

		if (msgs[i].flags & PLATFORM_I2C_RD) {
			st = HAL_I2C_Master_Receive(&hi2c1, hal_addr,
			                            msgs[i].buf, msgs[i].len,
			                            STM32_I2C_TIMEOUT_MS);
		} else {
			st = HAL_I2C_Master_Transmit(&hi2c1, hal_addr,
			                             msgs[i].buf, msgs[i].len,
			                             STM32_I2C_TIMEOUT_MS);
		}

		if (HAL_OK != st) {
			ret = (HAL_TIMEOUT == st) ? PLATFORM_ETIMEOUT : PLATFORM_EIO;
			goto exit;
		}
	}

exit:
	return ret;
}

static const struct platform_i2c_bus_device_ops _stm32_i2c_bus_ops = {
	.master_xfer = _stm32_i2c_master_xfer,
};

/* 这条 i2c 总线的实例. 应用层 led_cfg.c 在 !MOCK_PC 分支 extern 这个名字. */
struct platform_i2c_bus_device stm32_i2c1_bus;

static void _i2c_board_init(void)
{
	(void)platform_i2c_bus_register(&stm32_i2c1_bus, &_stm32_i2c_bus_ops);
}
INIT_BOARD_EXPORT(_i2c_board_init);
