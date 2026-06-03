/* SPDX-License-Identifier: MIT */
/*
 * main.c - 模拟 Linux 内核启动 + GPIO 子系统完整演示
 *
 * ┌─ 启动流程（模拟内核启动序列）────────────────────┐
 * │                                                   │
 * │  1. vendor_a_probe()  / vendor_b_probe()          │
 * │     ↓                                              │
 * │     内部调 gpiochip_add()，将两家芯片注册到        │
 * │     gpiolib 的全局芯片列表。                       │
 * │     （真实内核：module_init 阶段执行）              │
 * │                                                   │
 * │  2. gpio_get_desc("vendorA-gpio", 5)              │
 * │     ↓                                              │
 * │     按 label 查找 chip，返回 desc（chip + offset） │
 * │     （真实内核：device tree 解析阶段建立映射）      │
 * │                                                   │
 * │  3. led_gpio_brightness_set(led_red, 1)           │
 * │     ↓                                              │
 * │     gpiod_set_value(desc, 1)                      │
 * │     → desc->gc->set(..., 5, 1)                    │
 * │     → vendor_a_set(...)                            │
 * │     （多态 dispatch 完成）                          │
 * │                                                   │
 * │ 整个流程的关键：步骤 3 对两家芯片走不同实现路径。   │
 * └────────────────────────────────────────────────────┘
 *
 * ┌─ 演示目的 ────────────────────────────────────────┐
 * │ 这个 main.c 不是为了"做产品"，而是为了用最少的     │
 * │ 代码演示分层架构 + 多态 dispatch 的工作原理。      │
 * │ 看到终端输出中的 [vendorA] / [vendorB] 标签，      │
 * │ 你就知道同一段 leds-gpio 代码操作了不同的硬件。     │
 * └────────────────────────────────────────────────────┘
 *
 * ┌─ 调用链全景（以点亮红色 LED 为例） ────────────────┐
 * │                                                        │
 * │  main.c          leds_gpio.c    gpiolib.c     vendor_a │
 * │    │                │              │              │     │
 * │  vendor_a_probe() ─┼──────────────┼─ gpiochip_add ┼─→   │
 * │    │                │              │   注册 chip    │     │
 * │    │                │              │              │     │
 * │  gpio_get_desc() ──┼──────────────┼─ 遍历 s_chips │     │
 * │  ("vendorA-..")     │              │ → 匹配 label  │     │
 * │    │                │              │ → 分配 desc   │     │
 * │    │                │              │ → desc->gc =  │     │
 * │    │                │              │   &vendorA    │     │
 * │    │                │              │              │     │
 * │  led_gpio_bright ── gpiod_set ───── desc->gc->set ─→ vendor_a_set()
 * │  ness_set(desc,1)   value(desc,1)  (...,5,1)     │  打印 DR_REG    │
 * │    │                │              │              │  ←0x00000020   │
 * │                                                        │
 * │  另一路 led_green (desc->gc = &vendorB) 走同样的        │
 * │  路径，但最终落到 vendor_b_set()，打印 BSRR。           │
 * │  这就是"同一份代码，两家芯片"的本质。                    │
 * └────────────────────────────────────────────────────────┘
 */

#include "gpio_chip.h"
#include <stdio.h>
#include <stdlib.h>

/* 外部入口：厂商 probe 函数、LED 驱动接口 */
void vendor_a_probe(void);
void vendor_b_probe(void);
void led_gpio_brightness_set(struct gpio_desc *desc, int value);

int main(void)
{
	printf("=========================================\n");
	printf("  ch16 - linux-style gpio subsystem\n");
	printf("=========================================\n");

	/*
	 * 阶段1：启动期注册（模拟内核 module_init）
	 * 每家芯片厂商的 probe 函数将 chip 实例注册到 gpiolib。
	 */
	vendor_a_probe();
	vendor_b_probe();

	/*
	 * 阶段2：consumer 获取引脚描述符
	 * leds-gpio 驱动通过 chip label + offset 拿到 desc。
	 *   led_red   → vendorA-gpio 的第 5 引脚
	 *   led_green → vendorB-gpio 的第 2 引脚
	 *
	 * 注意：这里两个 desc 的 desc->gc 指向不同的 chip 实例。
	 * 这就是后续多态 dispatch 的数据基础。
	 */
	struct gpio_desc *led_red   = gpio_get_desc("vendorA-gpio", 5);
	struct gpio_desc *led_green = gpio_get_desc("vendorB-gpio", 2);

	/*
	 * 阶段3：驱动正常工作
	 * 同一段 leds-gpio 代码，操作两家芯片的引脚。
	 *
	 * 输出中会看到：
	 *   [vendorA] set offset=5 value=1 (DR_REG <- 0x00000020)
	 *   [vendorB] set offset=2 value=1 (BSRR <- 0x00000004)
	 *
	 * 同样是"点亮 LED"，两家芯片的寄存器操作截然不同。
	 * leds-gpio 驱动不知道也不关心这种差异。
	 */
	printf("\n--- leds-gpio drives both chips ---\n");
	led_gpio_brightness_set(led_red,   1);
	led_gpio_brightness_set(led_green, 1);
	led_gpio_brightness_set(led_red,   0);
	led_gpio_brightness_set(led_green, 0);

	printf("\n>>> same gpiod_set_value() dispatches to two vendors <<<\n");

	/* 释放动态分配的 desc（模拟内核 gpiod_put） */
	free(led_red);
	free(led_green);

	/*
	 * 等待输入再退出，方便在 Windows 下双击 exe 时
	 * 看到终端输出而不是一闪而过。
	 */
	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
