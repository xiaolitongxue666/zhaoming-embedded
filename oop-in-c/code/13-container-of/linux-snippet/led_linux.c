/* SPDX-License-Identifier: MIT */
/**
 * @file  led_linux.c
 * @brief ch13 Linux 用户态 sysfs 等效片段（函数式包装版 + container_of）
 *
 * @details
 * 本文件是 Linux 用户态真实实现片段。
 * 需要内核启用 sysfs gpio / pwm + i2c-dev 模块。
 * 用法: 把 gpio / pwm / i2c 节点 export 出来后, 用户态进程读写文件 /
 * ioctl 操作硬件。
 *
 * 父类 led_on / led_off / led_set_brightness 写在 led.c, 子类实现里
 * 第一行用 container_of 反推自己, 后面调 platform_gpio_xxx 封装函数 /
 * sysfs PWM / i2c-dev. Linux 用户态这一层落到真实文件读写,
 * 应用层 / 父类 / 子类一字不改.
 *
 * 见 ch13 § 13.10 在 Linux 用户态长什么样.
 */
#include "led.h"
#include "container_of.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

/* ============== platform_gpio_* (sysfs gpio 实现) ============== */

void platform_gpio_init(uint8_t pin, uint8_t mode)
{
	char path[64];
	int fd;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd >= 0) {
		dprintf(fd, "%u", (unsigned)pin);
		close(fd);
	}
	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%u/direction", (unsigned)pin);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		const char *dir = (mode == GPIO_MODE_OUTPUT) ? "out" : "in";
		write(fd, dir, strlen(dir));
		close(fd);
	}
}

void platform_gpio_deinit(uint8_t pin)
{
	int fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd >= 0) {
		dprintf(fd, "%u", (unsigned)pin);
		close(fd);
	}
}

void platform_gpio_write(uint8_t pin, bool value)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%u/value", (unsigned)pin);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, value ? "1" : "0", 1);
		close(fd);
	}
}

bool platform_gpio_read(uint8_t pin)
{
	char path[64], buf[2] = { 0 };
	int fd;

	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%u/value", (unsigned)pin);
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return false;
	read(fd, buf, 1);
	close(fd);
	return buf[0] == '1';
}

/* ============== PWM 子类 ops 在 Linux 上的实现 (含 set_brightness) ==
 *
 * sysfs pwmchip0/pwmN 节点 (CONFIG_PWM_SYSFS=y) 三个文件: period /
 * duty_cycle / enable. period 先于 duty_cycle 写, duty_cycle <= period.
 * enable 在 period / duty_cycle 都设好之后再写, 否则 -EINVAL.
 *
 * set_brightness 把 0..100 映射到 duty_cycle (0..period_ns), 实时改
 * sysfs 文件就能看到亮度变化.
 */

static int pwm_apply_duty(uint8_t channel, uint8_t duty)
{
	char path[64];
	int fd;
	unsigned long period_ns = 1000000UL;                  /* 1 kHz */
	unsigned long duty_ns   = period_ns * duty / 100UL;   /* 0..100 % */

	snprintf(path, sizeof(path),
		 "/sys/class/pwm/pwmchip0/pwm%u/period", (unsigned)channel);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;
	dprintf(fd, "%lu", period_ns);
	close(fd);

	snprintf(path, sizeof(path),
		 "/sys/class/pwm/pwmchip0/pwm%u/duty_cycle", (unsigned)channel);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;
	dprintf(fd, "%lu", duty_ns);
	close(fd);
	return 0;
}

static int pwm_apply_enable(uint8_t channel, bool on)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
		 "/sys/class/pwm/pwmchip0/pwm%u/enable", (unsigned)channel);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;
	write(fd, on ? "1" : "0", 1);
	close(fd);
	return 0;
}

int pwm_on_linux(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	pwm_apply_duty(self->channel, self->duty);
	pwm_apply_enable(self->channel, true);
	me->is_on = true;
	return 0;
}

int pwm_off_linux(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	pwm_apply_enable(self->channel, false);
	me->is_on = false;
	return 0;
}

int pwm_set_brightness_linux(struct led_base *me, uint8_t brightness)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	if (brightness > 100)
		brightness = 100;
	self->duty = brightness;
	pwm_apply_duty(self->channel, brightness);
	me->is_on = (brightness > 0);
	return 0;
}

/* ============== I2C 子类 ops 在 Linux 上的实现 ==============
 *
 * /dev/i2c-N 字符设备 (CONFIG_I2C_CHARDEV=y) + ioctl(I2C_SLAVE) 把
 * 7-bit 从机地址绑到 fd, 后续 write 走 START + addr<<1 + data + STOP.
 *
 * 错误路径: 任意一步失败立即 close fd 返回 -1.
 * cmd 字节是写到芯片寄存器 0 的值, 真实芯片的寄存器映射看 datasheet.
 */

static int i2c_send_byte(uint8_t bus, uint8_t addr, uint8_t cmd)
{
	char path[16];
	int fd;

	snprintf(path, sizeof(path), "/dev/i2c-%u", (unsigned)bus);
	fd = open(path, O_RDWR);
	if (fd < 0)
		return -1;
	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		close(fd);
		return -1;
	}
	if (write(fd, &cmd, 1) != 1) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int i2c_on_linux(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	if (i2c_send_byte(self->bus, self->addr, 0x01) < 0)
		return -1;
	me->is_on = true;
	return 0;
}

int i2c_off_linux(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	if (i2c_send_byte(self->bus, self->addr, 0x00) < 0)
		return -1;
	me->is_on = false;
	return 0;
}
