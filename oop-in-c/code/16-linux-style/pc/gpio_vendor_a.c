/* SPDX-License-Identifier: MIT */
/*
 * gpio_vendor_a.c - 厂商 A 的 GPIO 控制器驱动（硬件实现层）
 *
 * ┌─ 这家芯片的硬件特征 ──────────────────────────────────┐
 * │ 风格：DR_REG（Data Register）—— 一次写一个 32 位整字   │
 * │ 到数据寄存器，直接控制全部 32 引脚的电平。             │
 * │ 典型代表：NXP i.MX 系列 GPIO_DR 寄存器。              │
 * │                                                        │
 * │ 寄存器布局示意（PC 上用 printf 模拟）：                 │
 * │   [PORT_EN]  引脚功能使能                              │
 * │   [DIR]      方向控制                                  │
 * │   [DR]       数据寄存器（写整字控制多引脚电平）        │
 * └────────────────────────────────────────────────────────┘
 *
 * ┌─ 第1层视角（硬件实现层） ──────────────────────────────┐
 * │ 这一层唯一的工作：实现 gpio_chip 的 5 个函数指针。     │
 * │ 每个函数对应一段寄存器操作序列，芯片手册规定怎么写。   │
 * │ 厂商驱动工程师看的是《芯片参考手册》第 N 章寄存器描述。│
 * │                                                        │
 * │ 上层的 gpiolib 和 leds-gpio 完全不关心这里怎么写，     │
 * │ 它们只关心 gpio_chip 结构体上的 set 指针有没有挂函数。 │
 * │ 这就是"接口隔离"——第1层和第4层通过第2/3层对话。        │
 * │                                                        │
 * │ 真实内核等价物：drivers/gpio/gpio-mxc.c                │
 * │ 参见 ch16 §16.5                                        │
 * └────────────────────────────────────────────────────────┘
 *
 * ┌─ 第1层在五层架构中的位置 ──────────────────────────┐
 * │  第5层  main.c           初始化/演示                │
 * │  第4层  leds_gpio.c      设备驱动（不关心芯片）     │
 * │  第3层  gpiolib.c        核心框架（dispatch 接线）  │
 * │  第2层  gpio_chip.h      芯片抽象接口（虚基类）     │
 * │  第1层  gpio_vendor_a.c  ← 本文件：硬件实现层      │
 * │          gpio_vendor_b.c    另一家厂商的实现        │
 * └────────────────────────────────────────────────────┘
 */

#include "gpio_chip.h"
#include <stdio.h>

/* ── 实现 gpio_chip 的 5 个 ops 函数 ───────────────── */

static int vendor_a_request(struct gpio_chip *gc, unsigned int offset)
{
	/* 模拟写入 PORT_EN 寄存器，使能对应引脚的功能 */
	printf("    [vendorA] request offset=%u (write reg PORT_EN)\n",
	       offset);
	(void)gc;
	return 0;
}

static void vendor_a_free(struct gpio_chip *gc, unsigned int offset)
{
	/* 释放引脚（清 PORT_EN 对应位） */
	printf("    [vendorA] free offset=%u\n", offset);
	(void)gc;
}

static int vendor_a_direction_output(struct gpio_chip *gc,
				     unsigned int offset, int value)
{
	/*
	 * 设置引脚为输出模式（写 DIR 寄存器对应位 = 1）。
	 * 有些芯片在此处还会一并写初值到 DR。
	 */
	printf("    [vendorA] direction_output offset=%u (write reg DIR)\n",
	       offset);
	(void)gc;
	(void)value;
	return 0;
}

static int vendor_a_get(struct gpio_chip *gc, unsigned int offset)
{
	/* 读取引脚当前输入电平（读 PS / PSR 寄存器） */
	(void)gc;
	(void)offset;
	return 0;
}

static void vendor_a_set(struct gpio_chip *gc, unsigned int offset, int value)
{
	/*
	 * 厂商 A 风格：DR_REG（Data Register）一次写 32 位整字。
	 * 读-改-写三步：先读 DR 当前值 → 修改对应位 → 写回 DR。
	 * 这个操作是非原子的，在多任务环境下可能被中断打断。
	 *
	 * 与之对比，厂商 B 用 SET/CLR 寄存器实现原子操作。
	 * 这就是两个厂商实现同一个 gpio_chip 接口时，
	 * 内部细节可能完全不同的典型案例。
	 */
	printf("    [vendorA] set offset=%u value=%d (DR_REG <- 0x%08X)\n",
	       offset, value, value ? (1u << offset) : 0);
	(void)gc;
}

/* ── 初始化 chip 实例：填充"虚函数表" ───────────── */

/*
 * vendor_a_chip - 静态分配的 gpio_chip 实例
 *
 * 这就是"虚基类"的具体实例化。5 个函数指针各自指向
 * vendor 的实现函数，相当于 C++ 虚函数表的 vtable 填充。
 *
 * base = 0:   vendorA 占据全局 GPIO 编号 0-31
 * ngpio = 32: 提供 32 个 GPIO 引脚
 */
static struct gpio_chip vendor_a_chip = {
	.label            = "vendorA-gpio",
	.base             = 0,
	.ngpio            = 32,
	.request          = vendor_a_request,
	.free             = vendor_a_free,
	.direction_output = vendor_a_direction_output,
	.get              = vendor_a_get,
	.set              = vendor_a_set,
};

/* ── 入口函数（模拟内核 module_init） ────────────── */

void vendor_a_probe(void)
{
	/*
	 * 将填充好的 chip 实例注册到 gpiolib 子系统。
	 * 注册之后，consumer 就能通过 gpio_get_desc("vendorA-gpio", ...)
	 * 拿到 desc，然后通过 desc->gc->set 走到这里。
	 */
	gpiochip_add(&vendor_a_chip);
}
