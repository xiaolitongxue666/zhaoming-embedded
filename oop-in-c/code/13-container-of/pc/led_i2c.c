/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.c - I2C 子类 init + 实现层 + led_ops_i2c 操作表 (ch13 版)
 *
 * I2C 子类不填 set_brightness, 字段保持 NULL. ch14 把"字段为 NULL 怎么
 * 办"展开成必填 / 选填 / 全必填三种策略.
 */

#include "led_i2c.h"
#include "container_of.h"
#include <stdio.h>

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	printf("  [%s] I2C bus%u addr=0x%02X reg=0x01\n",
	       me->name, (unsigned)self->bus, (unsigned)self->addr);
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	printf("  [%s] I2C bus%u addr=0x%02X reg=0x00\n",
	       me->name, (unsigned)self->bus, (unsigned)self->addr);
	me->is_on = false;
	return 0;
}

static const struct led_ops led_ops_i2c = {
	.on  = i2c_on,
	.off = i2c_off,
	/* set_brightness 故意不填 -- I2C 简单 LED 也没有亮度概念 */
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
