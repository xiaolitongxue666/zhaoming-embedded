/* SPDX-License-Identifier: MIT */
/**
 * @file  led_i2c.c
 * @brief I2C 子类 init + 实现层 + led_ops_i2c 操作表
 *
 * @details
 * 三个 static 函数 i2c_on / i2c_off / i2c_toggle 在 PC 上用 printf
 * 模拟一次 "I2C write reg" 命令. 真机上这里调 platform_i2c_transfer
 * (ch15 二层 bus + client 接口, 本章不引入).
 *
 * 同样的 led_on(base), 应用层不知道也不需要知道这颗 LED 底下走的
 * 是拉电平 / 配 PWM / 还是 I2C 总线. 这就是多态.
 */

#include "led_i2c.h"
#include <stdio.h>

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
	printf("  [I2C] sub-class init done (addr=0x%02x, reg=0x%02x)\n",
	       (unsigned)dev_addr, (unsigned)reg);
	return 0;
}

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	me->is_on = true;
	printf("  [I2C] \"%s\" ON  (addr=0x%02x reg=0x%02x val=0x01)\n",
	       me->name, (unsigned)self->dev_addr, (unsigned)self->reg);
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	me->is_on = false;
	printf("  [I2C] \"%s\" OFF (addr=0x%02x reg=0x%02x val=0x00)\n",
	       me->name, (unsigned)self->dev_addr, (unsigned)self->reg);
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
