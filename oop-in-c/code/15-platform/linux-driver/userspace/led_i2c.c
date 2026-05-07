/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.c - 子类三: I2C 扩展芯片 LED 实现 (ch15 linux-driver/userspace 版).
 *
 * 写法: ioctl(I2C_SLAVE, addr) 让内核 i2c-dev 把后续 read / write 转给这个
 * 从机, 然后 write([reg, val]) 一次发两字节.
 *
 * 默认 val_on / val_off = 0x01 / 0x00, 适配大多数 IO expander 风格的 LED
 * 控制器 (PCA9554 / TCA6408 / PCA9555 之类). 不同芯片可在 init 之后再覆盖.
 */

#include "led_i2c.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define LED_I2C_DEFAULT_VAL_ON   0x01
#define LED_I2C_DEFAULT_VAL_OFF  0x00
#define LED_I2C_PATH_MAX         32

static int i2c_write_reg(struct led_i2c *self, uint8_t val)
{
	uint8_t buf[2];
	ssize_t n;

	buf[0] = self->reg;
	buf[1] = val;
	n = write(self->fd, buf, sizeof(buf));
	return (n != (ssize_t)sizeof(buf)) ? -1 : 0;
}

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	if (i2c_write_reg(self, self->val_on) < 0)
		return -1;
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	if (i2c_write_reg(self, self->val_off) < 0)
		return -1;
	me->is_on = false;
	return 0;
}

/* set_brightness 故意不填: I2C 这一路只控开 / 关, 走父类默认行为. */
static const struct led_ops i2c_ops = {
	.on  = i2c_on,
	.off = i2c_off,
};

int led_i2c_init(struct led_i2c *me, const char *name,
                 int bus_num, uint8_t dev_addr, uint8_t reg)
{
	char dev_path[LED_I2C_PATH_MAX];
	int  fd;

	if (!me || !name)
		return -1;

	snprintf(dev_path, sizeof(dev_path), "/dev/i2c-%d", bus_num);

	fd = open(dev_path, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "[led_i2c:%s] open %s failed: %s\n",
		        name, dev_path, strerror(errno));
		return -1;
	}
	if (ioctl(fd, I2C_SLAVE, dev_addr) < 0) {
		fprintf(stderr, "[led_i2c:%s] I2C_SLAVE 0x%02x failed: %s\n",
		        name, dev_addr, strerror(errno));
		close(fd);
		return -1;
	}

	me->fd       = fd;
	me->dev_addr = dev_addr;
	me->reg      = reg;
	me->val_on   = LED_I2C_DEFAULT_VAL_ON;
	me->val_off  = LED_I2C_DEFAULT_VAL_OFF;

	return led_base_init(&me->base, name, &i2c_ops);
}

void led_i2c_deinit(struct led_i2c *me)
{
	if (!me || me->fd < 0)
		return;
	close(me->fd);
	me->fd = -1;
}
