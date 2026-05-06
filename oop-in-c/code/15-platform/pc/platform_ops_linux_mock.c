/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_linux_mock.c - Linux 用户态"假装版"
 *
 * 真机上这一份会写 /sys/class/gpio/gpioN/value，触发内核 gpiolib 操作
 * 物理引脚。在 PC 上为了演示"运行时切换平台"，它只是把会执行的
 * shell 命令打到屏幕，标记 [LINUX]，让你看到"换到 Linux 时实际会
 * 跑什么"。
 *
 * 真实 Linux 工程的对应版本见 linux-snippet/platform_ops_linux.c。
 * 两份的接口签名一样，应用层一字不改。见 ch15 § 15.7。
 */

#include "platform_ops.h"
#include <stdio.h>

static void linux_gpio_init(uint8_t pin, uint8_t mode)
{
	const char *dir = (mode == 0) ? "out" : "in";
	printf("    [LINUX] echo %u > /sys/class/gpio/export\n",
	       (unsigned)pin);
	printf("    [LINUX] echo %s > /sys/class/gpio/gpio%u/direction\n",
	       dir, (unsigned)pin);
}

static void linux_gpio_write(uint8_t pin, bool value)
{
	printf("    [LINUX] echo %d > /sys/class/gpio/gpio%u/value\n",
	       value ? 1 : 0, (unsigned)pin);
}

static bool linux_gpio_read(uint8_t pin)
{
	(void)pin;
	return false;
}

static void linux_gpio_deinit(uint8_t pin)
{
	printf("    [LINUX] echo %u > /sys/class/gpio/unexport\n",
	       (unsigned)pin);
}

const struct platform_ops platform_linux_mock = {
	.name        = "LINUX",
	.gpio_init   = linux_gpio_init,
	.gpio_write  = linux_gpio_write,
	.gpio_read   = linux_gpio_read,
	.gpio_deinit = linux_gpio_deinit,
};
