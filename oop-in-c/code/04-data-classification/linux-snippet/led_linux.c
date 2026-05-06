/* SPDX-License-Identifier: MIT */
/*
 * led_linux.c - 数据归位完成后，LED 在 Linux 用户态长什么样
 *
 * 这是片段，不是完整工程。完整 Linux 工程见附录 C。
 *
 * 关键观察：led.h / led.c / main.c 一字不改。
 * Linux 用户态有充足的堆，本来用 malloc 也不会有问题，但本书
 * 坚持静态对象池的写法，理由是：
 *   1. 同一份代码 PC / STM32 / Linux 三平台跑
 *   2. 用静态池能让 valgrind / sanitizer 看不到任何堆分配
 *   3. 显式池容量比"堆能撑多少 LED"更可控
 */

#include "platform.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void platform_gpio_init(uint8_t pin, uint8_t mode)
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
 *   struct led *red = led_acquire(13);
 *   led_on(red);
 *   led_release(red);
 *
 * Linux 上 led_pool 在 .bss 段，进程启动时由 loader 清零。
 * 整个进程生命周期不需要任何 malloc。
 */
