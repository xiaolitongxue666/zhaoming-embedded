/* SPDX-License-Identifier: MIT */
/**
 * @file  led_linux.c
 * @brief 父类统一接口 led_on 落到 Linux 用户态 sysfs 的样子
 *
 * @details
 * 父类 led_on / led_off 写在 led.c, 子类实现走 platform_gpio_xxx
 * 封装函数. Linux 用户态这一层落到 /sys/class/gpio/ 文件读写,
 * 应用层 / 父类 / 子类 / board_init.c 一字不改.
 *
 * 跟 pc/ 唯一的差别就在这个文件: 把 printf 模拟换成真实 sysfs 操作.
 *
 * PWM 子类在真实 Linux 上走 /sys/class/pwm/, I2C 子类走 /dev/i2c-N
 * + ioctl. 这两路不在本片段里, 完整工程在附录 C.
 */
#include "led.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

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
