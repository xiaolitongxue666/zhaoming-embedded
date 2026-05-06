/* SPDX-License-Identifier: MIT */
/*
 * led_linux.c - 父类 / 子类 / 板级 / 应用 四层落到 Linux 用户态的样子
 *
 * ch15 完整框架在 Linux 用户态只换一份文件: 把 PC 模拟版的 4 个
 * platform 封装函数 (printf 模拟) 换成走 /sys/class/gpio 的真实
 * 文件读写. 父类 led.c / 子类 ops / 板级 board_init.c / 应用 app.c
 * 一字不动.
 *
 * 子类 gpio_on 里 platform_gpio_write(self->pin, self->on_level)
 * 在 Linux 用户态调到底就是 echo 1 > /sys/class/gpio/gpioN/value,
 * 走内核 gpiolib 操作物理引脚. 见 ch15 § 15.11 在 Linux 上长什么样.
 */

#include "platform.h"
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
	char path[64], buf[2] = {0};
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
