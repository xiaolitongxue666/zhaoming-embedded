/* SPDX-License-Identifier: MIT */
/*
 * gpio_chip.h - "山寨" 一份 Linux 内核 gpio_chip
 *
 * 把 ch15 的 platform_ops 改个名字、改成"按 chip 分组"的形态，
 * 你就得到了一份和 Linux 内核 GPIO 子系统 90% 相像的接口。
 *
 * 关键差别：之前的 platform 抽象是"全局一份 platform_ops 指针"，
 * 整个进程同一时刻只挂着一家芯片的实现。Linux 内核里同一颗 SoC 上
 * 可能并存多家厂商的 GPIO 控制器（片内 + 外扩 IO 扩展芯片），所以
 * gpio_chip 把每一份实现绑在一个 chip 实例上，desc->chip 反查 dispatch。
 * 这就是"再加一层抽象隔离芯片"的真实形态。
 *
 * 真实内核版定义在 include/linux/gpio/driver.h 第 415 行起，
 * 字段比这里多得多（中断、热插拔、debugfs 等），但骨架就是这样。
 *
 * 见 ch16 § 16.5 真实内核 + § 16.3 四层架构。
 */

#ifndef GPIO_CHIP_H
#define GPIO_CHIP_H

#include <stdint.h>
#include <stdbool.h>

struct gpio_chip {
	const char *label;     /* 芯片标识，gpio_get_desc 按 label 查 */
	uint32_t    base;      /* 这家芯片在全局 GPIO 编号空间的起始号 */
	uint32_t    ngpio;     /* 这家芯片提供的引脚数 */

	/* 下面这一组就是 ops 表：每家厂商往这些字段挂自己的实现函数。
	 * 相同的接口（5 个函数指针），不同的实现（不同寄存器布局）。
	 * gpiolib.c 通过 desc->gc 找到当前引脚归哪家管，调对应函数。
	 * 这就是 ch11 多态 dispatch 在 Linux GPIO 子系统里的形态。
	 */
	int  (*request)(struct gpio_chip *gc, unsigned int offset);
	void (*free)(struct gpio_chip *gc, unsigned int offset);
	int  (*direction_output)(struct gpio_chip *gc,
				 unsigned int offset, int value);
	int  (*get)(struct gpio_chip *gc, unsigned int offset);
	void (*set)(struct gpio_chip *gc, unsigned int offset, int value);

	void *driver_data;     /* 给具体 chip 实现挂自己的 context */
};

/* 内核态 gpio consumer 接口（简化版） */
struct gpio_desc {
	struct gpio_chip *gc;
	unsigned int      offset;
};

void gpiod_set_value(struct gpio_desc *desc, int value);
int  gpiod_get_value(struct gpio_desc *desc);

/* 注册 chip（真正内核里叫 gpiochip_add_data） */
int gpiochip_add(struct gpio_chip *gc);

/* 通过 chip + offset 拿 desc（真正内核走 device tree） */
struct gpio_desc *gpio_get_desc(const char *chip_label, unsigned int offset);

#endif /* GPIO_CHIP_H */
