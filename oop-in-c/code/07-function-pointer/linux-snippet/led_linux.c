/* SPDX-License-Identifier: MIT */
/*
 * led_linux.c - 函数指针变量在 Linux 用户态的样子
 *
 * 本章主线只演示独立函数指针变量本身 (见 ch07 § 7.4),
 * 没有把指针塞进任何 struct.
 *
 * Linux 上 gpio_on / gpio_off 走 sysfs gpio 接口, 实现和 ch01
 * linux-snippet 里的 platform_gpio_write 几乎一字不差.
 *
 * 把 gpio_on 的地址存进 fp, 通过 fp 调用, 行为和直接调 gpio_on
 * 等价. Linux 用户态的 branch predictor 比 MCU 强得多, 间接调用
 * 开销几乎看不出来.
 */

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

void gpio_on(uint8_t pin)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
	         "/sys/class/gpio/gpio%u/value", (unsigned)pin);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, "1", 1);
		close(fd);
	}
}

void gpio_off(uint8_t pin)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
	         "/sys/class/gpio/gpio%u/value", (unsigned)pin);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, "0", 1);
		close(fd);
	}
}

/*
 * 调用方 (节选):
 *
 *   void (*fp)(uint8_t);
 *   fp = gpio_on;
 *   fp(13);                 // 写 sysfs gpio13/value <- "1"
 *
 *   fp = gpio_off;
 *   fp(13);                 // 写 sysfs gpio13/value <- "0"
 *
 * 同一个 fp, 存不同函数地址, 拨通不同的实现.
 */
