/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_linux.c - 真实 Linux 用户态 platform_ops 实例（sysfs）
 *
 * pc/ 里 platform_ops_linux_mock.c 是 printf 假装的版本，方便在 PC
 * 上演示"运行时切换"。这一份是真实跑在 Linux 上的版本，底下走
 * /sys/class/gpio 文件接口，触发内核 gpiolib 操作物理引脚。
 *
 * 接口签名跟 pc/ 完全一样，led.c / app.c / board_init.c 一字不动。
 * 见 ch15 § 15.7 / 附录 C。
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

const struct platform_ops platform_linux = {
	.name       = "LINUX-real",
	.gpio_init  = linux_gpio_init,
	.gpio_write = linux_gpio_write,
	.gpio_read  = linux_gpio_read,
};

const struct platform_ops *platform = &platform_linux;
