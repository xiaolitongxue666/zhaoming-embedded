/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_linux.c - 真实 Linux 用户态 platform_ops 实例 (sysfs)
 *
 * 这一份只导出 const struct platform_ops platform_linux. 启动期 main
 * 调 platform_select(&platform_linux) 把 platform 层内部当前指针指向它.
 * 之后驱动层调 platform_gpio_xxx 封装函数, platform 层内部 dispatch 到
 * 这一份的具体实现.
 *
 * 跟 pc/platform_ops_pc.c 形态一致, 只是底下从 printf 换成
 * /sys/class/gpio/ 文件操作. 应用层 / led 层一字不改.
 */

#include "platform_ops.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define LINUX_GPIO_MODE_OUTPUT    0

static void linux_gpio_init(uint8_t pin, uint8_t mode)
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
		const char *dir = (mode == LINUX_GPIO_MODE_OUTPUT) ? "out" : "in";
		write(fd, dir, strlen(dir));
		close(fd);
	}
}

static void linux_gpio_deinit(uint8_t pin)
{
	int fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd >= 0) {
		dprintf(fd, "%u", (unsigned)pin);
		close(fd);
	}
}

static void linux_gpio_write(uint8_t pin, bool value)
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

static bool linux_gpio_read(uint8_t pin)
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

const struct platform_ops platform_linux = {
	.name        = "LINUX",
	.gpio_init   = linux_gpio_init,
	.gpio_deinit = linux_gpio_deinit,
	.gpio_write  = linux_gpio_write,
	.gpio_read   = linux_gpio_read,
};
