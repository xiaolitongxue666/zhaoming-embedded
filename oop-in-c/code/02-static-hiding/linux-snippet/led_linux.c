/* SPDX-License-Identifier: MIT */
/*
 * led_linux.c - 同一份 led_create / led_on 在 Linux 用户态长什么样
 *
 * 这是片段，不是完整工程。完整 Linux 工程见附录 C。
 * 用 sysfs gpio 暴露引脚（最朴素的方式，新内核推荐 libgpiod）。
 *
 * 关键观察：led.h / led.c / main.c 一字不改。
 * 字段定义还在 led.c 里，外部根本看不到 pin / brightness / is_on。
 */

#include "platform.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
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

/*
 * 应用层调用方式（和 PC / STM32 版完全一样）：
 *
 *   struct led *red = led_create(13);
 *   led_on(red);
 *   red->pin = 999;        // 编译报错: invalid use of undefined type
 *   led_destroy(red);
 *
 * 信息隐藏是编译期机制，跨平台行为一致。
 */
