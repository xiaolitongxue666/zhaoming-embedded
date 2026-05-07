/* SPDX-License-Identifier: MIT */
/*
 * led_i2c.c - LED I2C 子类实现 (基于 Linux i2c-dev 直接调用).
 *
 * val_on / val_off 默认 0x01 / 0x00, 适配大多数 IO expander 风格的 LED 控
 * 制器 (PCA9554 / TCA6408 之类). 不同芯片可在子类构造之后再覆盖.
 *
 * 写法是最朴素的"先选 slave 地址再 write 两字节": ioctl(I2C_SLAVE, addr)
 * 让内核 i2c-dev 把后续 read/write 转给这个从机, 然后 write([reg, val]).
 * 这是 i2c-dev 文档里的标准 SMBus byte write 等价写法.
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "drivers/led/led_i2c.h"

#define LED_I2C_DEFAULT_VAL_ON     0x01
#define LED_I2C_DEFAULT_VAL_OFF    0x00
#define LED_I2C_PATH_MAX           32

static platform_err_t _led_i2c_on(struct led_base *me);
static platform_err_t _led_i2c_off(struct led_base *me);

static const struct led_ops led_i2c_ops = {
	.on             = _led_i2c_on,
	.off            = _led_i2c_off,
	.set_brightness = NULL,    /* I2C LED 不支持调亮度, 走父类默认 no-op */
};

platform_err_t led_i2c_init(struct led_i2c *me, const char *name,
                            int bus_num, uint8_t dev_addr, uint8_t reg)
{
	char           dev_path[LED_I2C_PATH_MAX];
	int            fd;
	platform_err_t ret;

	if ((NULL == me) || (NULL == name)) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	(void)snprintf(dev_path, sizeof(dev_path), "/dev/i2c-%d", bus_num);

	fd = open(dev_path, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "[led_i2c:%s] open %s failed: %s\n",
		        name, dev_path, strerror(errno));
		ret = PLATFORM_EIO;
		goto exit;
	}

	if (ioctl(fd, I2C_SLAVE, dev_addr) < 0) {
		fprintf(stderr, "[led_i2c:%s] I2C_SLAVE 0x%02x failed: %s\n",
		        name, dev_addr, strerror(errno));
		(void)close(fd);
		ret = PLATFORM_EIO;
		goto exit;
	}

	me->fd       = fd;
	me->dev_addr = dev_addr;
	me->reg      = reg;
	me->val_on   = LED_I2C_DEFAULT_VAL_ON;
	me->val_off  = LED_I2C_DEFAULT_VAL_OFF;

	ret = led_base_init(&me->base, name, &led_i2c_ops);

exit:
	return ret;
}

void led_i2c_deinit(struct led_i2c *me)
{
	if ((NULL == me) || (me->fd < 0)) {
		return;
	}
	(void)close(me->fd);
	me->fd = -1;
}

static platform_err_t _led_i2c_write_reg(struct led_i2c *i2c, uint8_t val)
{
	uint8_t buf[2];
	ssize_t n;

	buf[0] = i2c->reg;
	buf[1] = val;

	n = write(i2c->fd, buf, sizeof(buf));
	if (n != (ssize_t)sizeof(buf)) {
		return PLATFORM_EIO;
	}
	return PLATFORM_EOK;
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
