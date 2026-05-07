/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.c - LED I2C 子类实现 (bus + client 二层).
 *
 * val_on / val_off 默认 0x01 / 0x00, 适配大多数 IO expander 风格的
 * LED 控制器 (PCA9554 / TCA6408 之类). 不同芯片可在子类构造之后再
 * 覆盖.
 *
 * 子类只调 platform_i2c_transfer ops 表层接口, 跨 MCU 0 改动. 一次
 * led_on / led_off 拼一段 msg (寄存器地址 + 写入值, 共 2 字节),
 * 走 platform_i2c_transfer 下到 master_xfer.
 */

#include "drivers/led/led_i2c.h"
#include <stddef.h>

#define LED_I2C_DEFAULT_VAL_ON     0x01
#define LED_I2C_DEFAULT_VAL_OFF    0x00

static int led_i2c_write(struct led_i2c *self, uint8_t value)
{
	uint8_t                  buf[2];
	struct platform_i2c_msg  msg;
	uint32_t                 done;

	buf[0] = self->reg;
	buf[1] = value;

	msg.addr  = self->client.client_addr;
	msg.flags = PLATFORM_I2C_WR;
	msg.len   = sizeof(buf);
	msg.buf   = buf;

	done = platform_i2c_transfer(self->client.bus, &msg, 1);
	return (done == 1) ? 0 : -1;
}

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	return led_i2c_write(self, LED_I2C_DEFAULT_VAL_ON);
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	return led_i2c_write(self, LED_I2C_DEFAULT_VAL_OFF);
}

static const struct led_ops i2c_ops = {
	.on             = i2c_on,
	.off            = i2c_off,
	.set_brightness = NULL,    /* I2C LED 不支持调亮度, 走父类默认 no-op */
};

int led_i2c_init(struct led_i2c *me, const char *name,
                 struct platform_i2c_bus_device *bus,
                 uint8_t client_addr, uint8_t reg)
{
	if (!me || !bus)
		return -1;

	me->client.bus         = bus;
	me->client.client_addr = client_addr;
	me->reg                = reg;

	return led_base_init(&me->base, name, &i2c_ops);
}
