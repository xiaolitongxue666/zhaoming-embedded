/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.c - 子类三: I2C 扩展芯片 LED 实现 (bus + client 二层)
 *
 * 这一份只负责 I2C 子类: 两件套 (i2c_on / i2c_off, set_brightness 故意
 * 不填走父类默认) + ops 表 (i2c_ops) + led_i2c_init 构造函数.
 *
 * 一次 led_on / led_off 拼一段 platform_i2c_msg (寄存器地址 + 写入值,
 * 共 2 字节), 走 platform_i2c_transfer(client.bus, &msg, 1) 下到 bus
 * 控制器层. 子类不直接 printf, 不直接碰 bus 控制器, 只通过 client 间接
 * 定位 -- bus 在 PC 端落到 platform_i2c_pc.c 的 stdout printf, 在 STM32
 * 端落到 platform/arch/stm32/pin_board.c 的 HAL_I2C_Master_Transmit,
 * 子类源码字节级不动.
 *
 * 见 ch15 § 15.17.2 / § 15.17.3 二层抽象 + § 15.11.5 跨 MCU.
 */

#include "led_i2c.h"
#include "container_of.h"
#include <stddef.h>
#include <stdio.h>

#define LED_I2C_VAL_ON     0x01
#define LED_I2C_VAL_OFF    0x00

static int led_i2c_write(struct led_i2c *self, uint8_t value)
{
	uint8_t                 buf[2];
	struct platform_i2c_msg msg;
	uint32_t                done;

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
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	int rc;

	printf("  [%s] led_on  -> I2C client_addr=0x%02X reg=0x%02X\n",
	       me->name, (unsigned)self->client.client_addr,
	       (unsigned)self->reg);
	rc = led_i2c_write(self, LED_I2C_VAL_ON);
	if (rc == 0)
		me->is_on = true;
	return rc;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	int rc;

	printf("  [%s] led_off -> I2C client_addr=0x%02X reg=0x%02X\n",
	       me->name, (unsigned)self->client.client_addr,
	       (unsigned)self->reg);
	rc = led_i2c_write(self, LED_I2C_VAL_OFF);
	if (rc == 0)
		me->is_on = false;
	return rc;
}

/* set_brightness 故意不填, I2C 这一路只控开/关, 走父类默认行为 */
static const struct led_ops i2c_ops = {
	.on  = i2c_on,
	.off = i2c_off,
};

int led_i2c_init(struct led_i2c *me, const char *name,
                 struct platform_i2c_bus_device *bus,
                 uint8_t client_addr, uint8_t reg)
{
	int rc;

	if (!me || !bus)
		return -1;
	rc = led_base_init(&me->base, name, &i2c_ops);
	if (rc != 0)
		return rc;

	me->client.bus         = bus;
	me->client.client_addr = client_addr;
	me->reg                = reg;
	return 0;
}
