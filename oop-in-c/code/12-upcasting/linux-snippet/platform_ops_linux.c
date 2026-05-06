/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_linux.c - Linux 用户态等效片段（sysfs）
 *
 * 把 ch12 PC 版的 platform_ops_pc.c 换成这一份，led.c / board_init.c / main.c
 * 一字不改。
 *
 * 完整 Linux 用户态工程（包括 libgpiod 版本）见附录 C。
 */

#include "platform_ops.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static void linux_gpio_init(uint8_t pin, uint8_t mode)
{
	int fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd >= 0) {
		dprintf(fd, "%u", (unsigned)pin);
		close(fd);
	}

	char path[64];
	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%u/direction", (unsigned)pin);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		const char *dir = (mode == GPIO_MODE_OUTPUT) ? "out" : "in";
		write(fd, dir, strlen(dir));
		close(fd);
	}
}

static void linux_gpio_write(uint8_t pin, bool value)
{
	char path[64];
	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%u/value", (unsigned)pin);
	int fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, value ? "1" : "0", 1);
		close(fd);
	}
}

static bool linux_gpio_read(uint8_t pin)
{
	char path[64], buf[2] = {0};
	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%u/value", (unsigned)pin);
	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return false;
	read(fd, buf, 1);
	close(fd);
	return buf[0] == '1';
}

static const struct platform_ops linux_ops = {
	.gpio_init  = linux_gpio_init,
	.gpio_write = linux_gpio_write,
	.gpio_read  = linux_gpio_read,
};

const struct platform_ops *platform = &linux_ops;
