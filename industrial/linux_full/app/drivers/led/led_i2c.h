/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.h - LED I2C 子类 (基于 Linux i2c-dev 实现).
 *
 * 直接走 /dev/i2c-N 字符设备 + ioctl(I2C_SLAVE) + write/read, 不再经过任何
 * platform 抽象层. i2c-dev 是 Linux 内核 i2c subsystem 暴露给用户态的标准
 * 接口, 用户态再套一层是过度封装.
 *
 * 这一份演示"换硬件不改应用": 同样的 led_on / led_off, 子类底下从拉 GPIO
 * 换成走 I2C 总线写控制寄存器. 上层应用 / 业务层一字不动.
 *
 * 不实现 set_brightness, 走父类默认 no-op.
 */

#ifndef DRIVERS_LED_LED_I2C_H_
#define DRIVERS_LED_LED_I2C_H_

#include <stdint.h>

#include "drivers/led/led_base.h"
#include "led_errors.h"

struct led_i2c {
	struct led_base base;
	int             fd;          /* open("/dev/i2c-<bus_num>") */
	uint8_t         dev_addr;    /* I2C 7 位从设备地址 */
	uint8_t         reg;         /* 控制寄存器地址 */
	uint8_t         val_on;      /* 写入这个值表示 ON (默认 0x01) */
	uint8_t         val_off;     /* 写入这个值表示 OFF (默认 0x00) */
};

/* 构造函数.
 *   me         子类实例
 *   name       实例名
 *   bus_num    /dev/i2c-<bus_num> (树莓派 4B 板上 I2C 一般是 1)
 *   dev_addr   I2C 7 位从设备地址
 *   reg        控制寄存器地址
 *
 * 内部调用顺序:
 *   1. open("/dev/i2c-<bus_num>", O_RDWR)
 *   2. ioctl(fd, I2C_SLAVE, dev_addr)
 *   3. led_base_init 填 ops 表
 *
 * 失败 PLATFORM_EIO. 板子没启用 I2C / 用户没在 i2c group / 设备地址 ack 不上,
 * 都是这一个错.
 */
platform_err_t led_i2c_init(struct led_i2c *me, const char *name,
                            int bus_num, uint8_t dev_addr, uint8_t reg);

/* 关 fd. */
void led_i2c_deinit(struct led_i2c *me);

#endif /* DRIVERS_LED_LED_I2C_H_ */
