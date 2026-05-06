/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_linux.c - ch13 Linux 用户态 sysfs 等效片段
 *
 * 把 ch13 pc/ 下的 4 个 platform 封装函数（platform_gpio_init / write /
 * read / deinit）替换到这一份实现：底层走 /sys/class/gpio 的 export +
 * direction + value 文件接口。led.c / main.c / container_of.h 一字不动。
 *
 * 这就是 platform 抽象的兑现：上层只调封装函数签名，换平台 = 换这一份
 * 文件，其它代码 0 修改。见 ch13 § 13.10 + ch15 § 15.5。
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
