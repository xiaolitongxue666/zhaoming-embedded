/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.h - 子类三: I2C 扩展芯片 LED (ch15 linux-driver/userspace 版).
 *
 * 直接走 /dev/i2c-N 字符设备 + ioctl(I2C_SLAVE) + write 两字节. 没有
 * platform_i2c_xxx 中间层 -- 内核 i2c-dev 已经做完.
 */

#ifndef LED_I2C_H
#define LED_I2C_H

#include "led_base.h"

struct led_i2c {
	struct led_base base;        /* 父类, 第 0 字段 */
	int             fd;          /* /dev/i2c-N 文件描述符 */
	uint8_t         dev_addr;    /* 7-bit slave addr */
	uint8_t         reg;         /* 写入的寄存器编号 */
	uint8_t         val_on;      /* on 时写的字节 */
	uint8_t         val_off;     /* off 时写的字节 */
};

int  led_i2c_init(struct led_i2c *me, const char *name,
                  int bus_num, uint8_t dev_addr, uint8_t reg);
void led_i2c_deinit(struct led_i2c *me);

#endif /* LED_I2C_H */
