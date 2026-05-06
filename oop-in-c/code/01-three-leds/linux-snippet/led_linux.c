/* SPDX-License-Identifier: MIT */
/*
 * led_linux.c - 同一份 led_on / led_off 在 Linux 用户态长什么样
 *
 * 这是片段，不是完整工程。完整 Linux 工程见附录 C。
 * 用 sysfs gpio 暴露引脚（最直观的方式，新内核推荐 libgpiod）。
 *
 * 关键观察：和 STM32 版一样 —— led.h / led.c / main.c 一字不改。
 * 变化的只是 platform_* 这层薄薄的胶水。
 */

#include "led.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/*
 * sysfs gpio 接口 (Linux 4.8 之前推荐, 新内核推荐 libgpiod):
 *   echo 13 > /sys/class/gpio/export
 *   echo out > /sys/class/gpio/gpio13/direction
 *   echo 1 > /sys/class/gpio/gpio13/value
 *
 * 把 platform.h 的接口接到这些 sysfs 文件上。
 */

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
 * 应用层调用方式：
 *
 *   struct led red_led;
 *   led_init(&red_led, 13);
 *   led_on(&red_led);     <-- 和 PC / STM32 版完全一样
 *
 * 编译命令：
 *
 *   gcc -Wall -Wextra -std=c99 -I../../common \
 *       -o led_demo main.c led.c led_linux.c
 *
 * 运行（树莓派或其它 Linux SBC，需 root 才能写 sysfs）：
 *
 *   sudo ./led_demo
 *
 * 现代 Linux (5.x+) 更推荐 libgpiod 接口，调用方式见附录 C。
 */
