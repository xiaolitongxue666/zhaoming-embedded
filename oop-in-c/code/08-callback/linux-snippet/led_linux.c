/* SPDX-License-Identifier: MIT */
/*
 * led_linux.c - 函数指针当参数 在 Linux 用户态的样子
 *
 * 三种 on/off 实现走真实操作: GPIO 写 sysfs / PWM 写 sysfs duty_cycle /
 * I2C 走 i2c-dev 发命令. 三个函数签名都是 void name(int param), 能给
 * 同一个 test_led 当函数指针参数.
 *
 * 第三个参数在不同实现里含义不同: GPIO 是引脚号, PWM 是通道号, I2C
 * 是从机地址. 由调用方负责传匹配的 id.
 */

#include "led.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

void gpio_on(int pin)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
	         "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, "1", 1);
		close(fd);
	}
}

void gpio_off(int pin)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
	         "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, "0", 1);
		close(fd);
	}
}

void pwm_on(int channel)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
	         "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle", channel);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, "1000000", 7);          /* duty 100% (period = 1000000ns) */
		close(fd);
	}
	snprintf(path, sizeof(path),
	         "/sys/class/pwm/pwmchip0/pwm%d/enable", channel);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, "1", 1);
		close(fd);
	}
}

void pwm_off(int channel)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
	         "/sys/class/pwm/pwmchip0/pwm%d/enable", channel);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, "0", 1);
		close(fd);
	}
}

void i2c_on(int addr)
{
	int fd = open("/dev/i2c-1", O_RDWR);
	uint8_t cmd[1] = { 0x01 };

	if (fd < 0)
		return;
	if (ioctl(fd, I2C_SLAVE, addr) >= 0)
		write(fd, cmd, sizeof(cmd));
	close(fd);
}

void i2c_off(int addr)
{
	int fd = open("/dev/i2c-1", O_RDWR);
	uint8_t cmd[1] = { 0x00 };

	if (fd < 0)
		return;
	if (ioctl(fd, I2C_SLAVE, addr) >= 0)
		write(fd, cmd, sizeof(cmd));
	close(fd);
}
