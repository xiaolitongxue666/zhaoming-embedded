/* SPDX-License-Identifier: MIT */
/*
 * sysfs_gpio_real.c - Linux 用户态 GPIO 在真实环境怎么写
 *
 * 这是片段，用来对比"教学版 PC printf vs 真实 sysfs"。完整 Linux 工程见附录 C。
 *
 * sysfs gpio 接口（Linux 4.8 之前推荐，新内核推荐 libgpiod）：
 *   echo 13 > /sys/class/gpio/export
 *   echo out > /sys/class/gpio/gpio13/direction
 *   echo 1 > /sys/class/gpio/gpio13/value
 *
 * Linux 用户态没有 STM32 那种 BSRR 寄存器直写。从用户态到硬件
 * 的路径是：
 *   write("/sys/class/gpio/gpio13/value", "1")
 *      -> kernel 调 gpio_chip 的 .set 方法
 *      -> 具体 GPIO controller driver
 *      -> 最终还是 BSRR / 类似的硬件寄存器
 *
 * 第 16 章我们会讲这个 chain：sysfs 的"把硬件当文件"机制怎么和
 * 内核的 file_operations + gpio_chip 衔接起来。
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

static int gpio_export(unsigned pin)
{
	int fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0)
		return -1;
	dprintf(fd, "%u", pin);
	close(fd);
	return 0;
}

static int gpio_set_dir(unsigned pin, const char *dir)
{
	char path[64];
	snprintf(path, sizeof(path),
	         "/sys/class/gpio/gpio%u/direction", pin);

	int fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;
	write(fd, dir, strlen(dir));
	close(fd);
	return 0;
}

static int gpio_write(unsigned pin, int value)
{
	char path[64];
	snprintf(path, sizeof(path),
	         "/sys/class/gpio/gpio%u/value", pin);

	int fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;
	write(fd, value ? "1" : "0", 1);
	close(fd);
	return 0;
}

void blink_demo(void)
{
	gpio_export(13);
	gpio_set_dir(13, "out");

	gpio_write(13, 1);
	sleep(1);
	gpio_write(13, 0);
}

/*
 * 命名规范的对照：
 *   ch03 学过：led_xxx 前缀 = 类名
 *   sysfs 用法：每个引脚走 /sys/class/gpio/gpioN/<attr> 这种路径前缀，
 *              attr = direction / value / edge / active_low
 *   本质是同一个 namespace 思路，只是 sysfs 用文件路径，C 模块用前缀。
 *
 * ch02 信息隐藏的对照：
 *   sysfs 文件就是菜单（用户态能 read/write 的属性）
 *   gpio_chip 内部的 .request / .set / .get 函数指针就是后厨
 *   用户态根本不知道底下是哪个 GPIO controller。
 */
