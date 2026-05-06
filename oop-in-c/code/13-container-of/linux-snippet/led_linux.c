/* SPDX-License-Identifier: MIT */
/**
 * @file  led_linux.c
 * @brief ch13 Linux 用户态 sysfs 等效片段（函数式包装版）
 *
 * @details
 * 父类 led_on / led_off / led_set_brightness 写在 led.c, 子类实现里
 * 第一行用 container_of 反推自己, 后面调 platform_gpio_xxx 封装函数.
 * Linux 用户态这一层落到 /sys/class/gpio/ 文件读写, 应用层 / 父类 /
 * 子类一字不改.
 *
 * 跟 pc/ 唯一的差别就在这个文件: 把 printf 模拟换成真实 sysfs 操作.
 *
 * 注: 这里是 ch01-ch10 沿用的函数式包装教学简化版. ch15 (Platform 层)
 * 会重构成 ops 表 (虚函数表) 形式, 和工业代码对齐.
 *
 * 见 ch13 § 13.10 在 Linux 用户态长什么样.
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
