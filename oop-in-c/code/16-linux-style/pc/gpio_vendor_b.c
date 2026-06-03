/* SPDX-License-Identifier: MIT */
/*
 * gpio_vendor_b.c - 厂商 B 的 GPIO 控制器驱动（硬件实现层）
 *
 * ┌─ 这家芯片的硬件特征（与厂商 A 的关键差异）───────────┐
 * │ 风格：SET / CLR 双寄存器（类似 STM32 的 BSRR）       │
 * │                                                        │
 * │   SET 寄存器（低 16 位）：写 1 到 offset 位 = 置高    │
 * │   CLR 寄存器（高 16 位）：写 1 到 offset+16 位 = 置低 │
 * │                                                        │
 * │ 与 vendor A 的 DR_REG 相比，BSRR 风格的优点是：        │
 * │   - 原子操作：一次 store，不需要读-改-写               │
 * │   - 不会因中断打断而丢失中间状态                       │
 * │   - 同一指令可同时置高某些引脚 + 置低另一些            │
 * │                                                        │
 * │ 典型代表：STM32 GPIO_BSRR、TI OMAP 的 GPIO_SETDATAOUT  │
 * └────────────────────────────────────────────────────────┘
 *
 * ┌─ 为什么要有两家厂商？ ────────────────────────────────┐
 * │ 这是 ch16 的演示核心：同一份 gpiod_set_value() 调用，  │
 * │ 根据 desc->gc 指向哪家 chip，走到不同的 set 实现。    │
 * │ 上层驱动 (leds_gpio.c) 完全不知道这种差异。           │
 * │ 这就是"1 (接口) × N (芯片) → 1 + N (分层)"的魔力。   │
 * │ 参见 ch16 §16.4、§16.5                                 │
 * └────────────────────────────────────────────────────────┘
 *
 * ┌─ 第1层在五层架构中的位置 ──────────────────────────┐
 * │  第5层  main.c           初始化/演示                │
 * │  第4层  leds_gpio.c      设备驱动（不关心芯片）     │
 * │  第3层  gpiolib.c        核心框架（dispatch 接线）  │
 * │  第2层  gpio_chip.h      芯片抽象接口（虚基类）     │
 * │  第1层  gpio_vendor_b.c  ← 本文件：硬件实现层      │
 * │          gpio_vendor_a.c    另一家厂商的实现        │
 * └────────────────────────────────────────────────────┘
 */

#include "gpio_chip.h"
#include <stdio.h>

/* ── 实现 gpio_chip 的 5 个 ops 函数 ───────────────── */

static int vendor_b_request(struct gpio_chip *gc, unsigned int offset)
{
	/*
	 * 厂商 B 在 request 时清除 LOCK 寄存器。
	 * 某些芯片硬件上有引脚锁定机制防止误改写，
	 * request 时先解锁。vendor_a 没有这个步骤。
	 */
	printf("    [vendorB] request offset=%u (clear reg LOCK)\n", offset);
	(void)gc;
	return 0;
}

static void vendor_b_free(struct gpio_chip *gc, unsigned int offset)
{
	printf("    [vendorB] free offset=%u\n", offset);
	(void)gc;
}

static int vendor_b_direction_output(struct gpio_chip *gc,
				     unsigned int offset, int value)
{
	/*
	 * 设置 MODE 寄存器。不同厂商的叫法不同：
	 * vendor A 叫 DIR，vendor B 叫 MODE，意义相同。
	 * 两家的硬件寄存器命名不同，但 gpio_chip 接口是同一套。
	 */
	printf("    [vendorB] direction_output offset=%u (set reg MODE)\n",
	       offset);
	(void)gc;
	(void)value;
	return 0;
}

static int vendor_b_get(struct gpio_chip *gc, unsigned int offset)
{
	(void)gc;
	(void)offset;
	return 0;
}

static void vendor_b_set(struct gpio_chip *gc, unsigned int offset, int value)
{
	/*
	 * 厂商 B 用 SET / CLR 双寄存器（类似 STM32 BSRR）。
	 *
	 * 高 16 位 = CLR（写 1 = 输出低电平）
	 * 低 16 位 = SET（写 1 = 输出高电平）
	 *
	 * 一次 32 位 store 即可完成操作，不需要读-改-写，
	 * 在多任务环境下是原子的，不会因为中断而丢失状态。
	 * 这是与 vendor A 的 DR_REG 风格最核心的区别。
	 */
	uint32_t reg = value ? (1u << offset) : (1u << (offset + 16));
	printf("    [vendorB] set offset=%u value=%d (BSRR <- 0x%08X)\n",
	       offset, value, reg);
	(void)gc;
}

/* ── 初始化 chip 实例 ────────────────────────────── */

/*
 * vendor_b_chip - 模拟"外扩 IO 扩展芯片"
 *
 * 注意 base = 32 和 ngpio = 16 的用意：
 *   - 片内 GPIO 控制器（vendor A）：base=0, ngpio=32 → 编号 0-31
 *   - 外扩 IO 扩展芯片（vendor B）：base=32, ngpio=16 → 编号 32-47
 *
 * 两块芯片共享全局编号空间，各自管理自己的内部偏移。
 * 真实 SoC 中常见的场景：片内控制器 + I2C/SPI 扩展 GPIO 共存。
 */
static struct gpio_chip vendor_b_chip = {
	.label            = "vendorB-gpio",
	.base             = 32,
	.ngpio            = 16,
	.request          = vendor_b_request,
	.free             = vendor_b_free,
	.direction_output = vendor_b_direction_output,
	.get              = vendor_b_get,
	.set              = vendor_b_set,
};

/* ── 入口函数（模拟内核 module_init） ────────────── */

void vendor_b_probe(void)
{
	gpiochip_add(&vendor_b_chip);
}
