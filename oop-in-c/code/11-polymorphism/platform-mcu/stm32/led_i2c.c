/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.c - LED I2C 子类 STM32 端真机实现
 *
 * 这是子类内部的 STM32 真机版本 (片段, 不是完整工程). 完整 STM32 工程
 * 见附录 B (Zephyr v3.7.0 LTS · stm32f4_disco).
 *
 * I2C LED 真实硬件场景: PCA9554 / TCA6408 这类 IO expander 通过 I2C
 * 拓展更多通道, 主控只发一次写寄存器命令就开 / 关一颗 LED. pc/ 端用
 * printf 模拟, STM32 端调 HAL_I2C_Master_Transmit 走真实 I2C 总线.
 *
 * 子类 init 签名 / ops 表形态跟 pc/ 一字不变, 只把模拟动作换成 HAL.
 * 应用层 / 父类一行不改.
 *
 * 真实工程里 I2C 句柄通过 CubeMX 生成的全局变量拿到 (hi2c1), 这里片段
 * 用 extern 声明引用. addr << 1 是因为 HAL_I2C 接的是 8 位地址 (7 位
 * 地址 + R/W 位), 调用方传的是标准 7 位地址.
 */

#include "led_i2c.h"
#include "stm32f4xx_hal.h"

/* CubeMX 生成的外设句柄 (I2C 走 I2C1). */
extern I2C_HandleTypeDef hi2c1;

static const struct led_ops led_ops_i2c;

int led_i2c_init(struct led_i2c *me, const char *name,
                 uint8_t dev_addr, uint8_t reg)
{
	int rc;
	if (!me)
		return -1;
	rc = led_base_init(&me->base, name, &led_ops_i2c);
	if (rc != 0)
		return rc;
	me->dev_addr = dev_addr;
	me->reg = reg;
	return 0;
}

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	uint8_t cmd[2] = { self->reg, 0x01 };

	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(self->dev_addr << 1),
	                        cmd, sizeof(cmd), 100);
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	uint8_t cmd[2] = { self->reg, 0x00 };

	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(self->dev_addr << 1),
	                        cmd, sizeof(cmd), 100);
	me->is_on = false;
	return 0;
}

static int i2c_toggle(struct led_base *me)
{
	if (me->is_on)
		return i2c_off(me);
	return i2c_on(me);
}

static const struct led_ops led_ops_i2c = {
	.on     = i2c_on,
	.off    = i2c_off,
	.toggle = i2c_toggle,
};
