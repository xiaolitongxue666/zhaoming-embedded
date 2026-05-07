/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.c - LED I2C 子类实现.
 *
 * val_on / val_off 默认 0x01 / 0x00, 适配大多数 IO expander 风格的 LED 控
 * 制器 (PCA9554 / TCA6408 之类). 不同芯片可在子类构造之后再覆盖.
 *
 * on / off 各拼一段 platform_i2c_msg (2 字节: reg + value), 走
 * platform_i2c_transfer 发出去. 跟 EEPROM 写命令模式一致.
 */

#include <stddef.h>

#include "drivers/led/led_i2c.h"
#include "platform/platform_i2c.h"

#define LED_I2C_DEFAULT_VAL_ON     0x01
#define LED_I2C_DEFAULT_VAL_OFF    0x00

static platform_err_t _led_i2c_on(struct led_base *me);
static platform_err_t _led_i2c_off(struct led_base *me);
static platform_err_t _led_i2c_write_reg(struct led_i2c *i2c, uint8_t value);

static const struct led_ops led_i2c_ops = {
	.on             = _led_i2c_on,
	.off            = _led_i2c_off,
	.set_brightness = NULL,    /* I2C LED 不支持调亮度, 走父类默认 no-op */
};

platform_err_t led_i2c_init(struct led_i2c *me, const char *name,
                            struct platform_i2c_bus_device *bus,
                            uint16_t client_addr, uint8_t reg)
{
	platform_err_t ret;

	if ((NULL == me) || (NULL == bus)) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	me->client.bus         = bus;
	me->client.client_addr = client_addr;
	me->reg                = reg;
	me->val_on             = LED_I2C_DEFAULT_VAL_ON;
	me->val_off            = LED_I2C_DEFAULT_VAL_OFF;

	ret = led_base_init(&me->base, name, &led_i2c_ops);

exit:
	return ret;
}

static platform_err_t _led_i2c_on(struct led_base *me)
{
	struct led_i2c *i2c = (struct led_i2c *)me;

	return _led_i2c_write_reg(i2c, i2c->val_on);
}

static platform_err_t _led_i2c_off(struct led_base *me)
{
	struct led_i2c *i2c = (struct led_i2c *)me;

	return _led_i2c_write_reg(i2c, i2c->val_off);
}

/* 拼 1 段 msg (2 字节: reg 地址 + value), 走 platform_i2c_transfer 发出去.
 * 等价于 i2c_smbus_write_byte_data(client_addr, reg, value). */
static platform_err_t _led_i2c_write_reg(struct led_i2c *i2c, uint8_t value)
{
	uint8_t                  buf[2];
	struct platform_i2c_msg  msg;

	buf[0] = i2c->reg;
	buf[1] = value;

	msg.addr  = i2c->client.client_addr;
	msg.flags = PLATFORM_I2C_WR;
	msg.len   = 2;
	msg.buf   = buf;

	return platform_i2c_transfer(i2c->client.bus, &msg, 1);
}
