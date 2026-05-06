/* SPDX-License-Identifier: MIT */
/*
 * ch14 Linux 用户态等效片段（sysfs）：替换 ch14 pc/ 里 PC 模拟版的
 * 4 个 platform 封装函数实现。led.c / main.c / container_of.h 一字不动。
 *
 * 必填 / 选填 / 接口三种 ops 表策略和 platform 在哪个平台无关——assert
 * 在 Linux 用户态调试构建里照样 abort。
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
