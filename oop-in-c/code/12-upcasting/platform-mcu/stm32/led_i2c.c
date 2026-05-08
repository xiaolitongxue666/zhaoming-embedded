/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.c - LED I2C 子类 STM32 端真机实现 (ch12 版)
 *
 * 这是子类内部的 STM32 真机版本 (片段, 不是完整工程). 完整 STM32 工程
 * 见附录 B (Zephyr v3.7.0 LTS · stm32f4_disco).
 *
 * I2C LED 真实硬件场景: PCA9554 / TCA6408 这类 IO expander 通过 I2C
 * 拓展更多通道, 主控只发一次写寄存器命令就开 / 关一颗 LED.
 *
 * HAL_I2C_Master_Transmit 一次发 1 字节命令: 0x01 = ON, 0x00 = OFF.
 * 厂家协议简化版, 真实芯片协议 (PCA9555 等) 命令字位宽和寄存器布局不
 * 一样, 替换 cmd 数组内容即可. addr << 1 是因为 HAL_I2C 接的是 8 位
 * 地址 (7 位地址 + R/W 位), 调用方传的是标准 7 位地址.
 *
 * 真实工程里 I2C 句柄通过 CubeMX 生成的全局变量拿到 (hi2c1), 这里片段
 * 用 extern 声明引用.
 */

#include "led_i2c.h"
#include "stm32f4xx_hal.h"

/* CubeMX 生成的外设句柄 (I2C 走 I2C1). */
extern I2C_HandleTypeDef hi2c1;

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	uint8_t cmd[1] = { 0x01 };

	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(self->addr << 1),
	                        cmd, sizeof(cmd), 100);
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	uint8_t cmd[1] = { 0x00 };

	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(self->addr << 1),
	                        cmd, sizeof(cmd), 100);
	me->is_on = false;
	return 0;
}

static const struct led_ops led_ops_i2c = {
	.on  = i2c_on,
	.off = i2c_off,
};

int led_i2c_init(struct led_i2c *me, const char *name,
                 uint8_t bus, uint8_t addr)
{
	int rc;
	if (!me)
		return -1;
	rc = led_base_init(&me->base, name, &led_ops_i2c);
	if (rc != 0)
		return rc;
	me->bus  = bus;
	me->addr = addr;
	return 0;
}
