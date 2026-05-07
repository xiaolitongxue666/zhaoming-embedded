/* SPDX-License-Identifier: MIT */
/**
 * @file  led_i2c.c
 * @brief I2C 子类 init + 实现层 + led_ops_i2c 操作表 (ch12 版)
 *
 * @details
 * i2c_on / i2c_off 函数签名都是 (struct led_base *me). 第一行
 * (struct led_i2c *)me 强转回子类拿 bus / addr 字段, 合法因为 base
 * 在 led_i2c 的第 0 字段.
 *
 * PC 教学版用 printf 模拟一次 I2C 写寄存器命令. 真机上这里调
 * platform_i2c_transfer (ch15 二层 bus + client 接口, 本章不引入).
 */

#include "led_i2c.h"
#include <stdio.h>

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	printf("  [%s] I2C bus%u addr=0x%02X reg=0x01\n",
	       me->name, (unsigned)self->bus, (unsigned)self->addr);
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	printf("  [%s] I2C bus%u addr=0x%02X reg=0x00\n",
	       me->name, (unsigned)self->bus, (unsigned)self->addr);
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
	me->bus = bus;
	me->addr = addr;
	return 0;
}
