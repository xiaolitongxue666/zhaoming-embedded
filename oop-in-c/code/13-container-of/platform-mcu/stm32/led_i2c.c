/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.c - LED I2C 子类 STM32 端真机实现 (ch13 版)
 *
 * 这是子类内部的 STM32 真机版本 (片段, 不是完整工程). 完整 STM32 工程
 * 见附录 B (industrial/stm32_full).
 *
 * I2C 子类 base 在第 0 字段, container_of 偏移为 0 编译器会把减法优化
 * 掉. 跟 GPIO 子类的 base 偏移 4 形成对比, container_of 一份代码两套
 * 偏移都对.
 *
 * HAL_I2C_Master_Transmit 一次发 1 字节命令: 0x01 = ON, 0x00 = OFF.
 * addr << 1 是因为 HAL_I2C 接的是 8 位地址 (7 位地址 + R/W 位).
 *
 * 真实工程里 I2C 句柄通过 CubeMX 生成的全局变量拿到 (hi2c1), 这里片段
 * 用 extern 声明引用.
 */

#include "led_i2c.h"
#include "container_of.h"
#include "stm32f4xx_hal.h"

/* CubeMX 生成的外设句柄 (I2C 走 I2C1). */
extern I2C_HandleTypeDef hi2c1;

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	uint8_t cmd[1] = { 0x01 };

	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(self->addr << 1),
	                        cmd, sizeof(cmd), 100);
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	uint8_t cmd[1] = { 0x00 };

	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(self->addr << 1),
	                        cmd, sizeof(cmd), 100);
	me->is_on = false;
	return 0;
}

const struct led_ops led_ops_i2c = {
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
