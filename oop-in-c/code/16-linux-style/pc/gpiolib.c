/* SPDX-License-Identifier: MIT */
/*
 * gpiolib.c - GPIO 核心框架层（芯片注册 + consumer API 实现）
 *
 * ┌─ 这一层的职责 ────────────────────────────────────────┐
 * │ 1. 维护已注册 chip 列表（s_chips[]）                   │
 * │ 2. 提供 chip 注册接口 (gpiochip_add)                  │
 * │ 3. 实现 consumer API (gpiod_set_value / get_value)    │
 * │ 4. 实现 chip 查找接口 (gpio_get_desc)                 │
 * │                                                        │
 * │ 这一层是"接线工"——它不控制硬件，它知道"谁控制硬件"。   │
 * │ consumer 说"设置引脚"，gpiolib 负责找到该引脚所属的    │
 * │ chip 实例，然后调 chip->set。                          │
 * │                                                        │
 * │ 这就是分层架构中"中间层"的价值：把 N (芯片) 和         │
 * │ M (设备驱动) 解耦，让它们不直接依赖对方。               │
 * └────────────────────────────────────────────────────────┘
 *
 * 真实内核：drivers/gpio/gpiolib.c
 * 关键路径：gpiod_set_value → gpiod_set_value_nocheck
 *         → gpiod_set_raw_value_commit → gc->set(...)
 * 参见 ch16 §16.5、ch11 多态 dispatch
 */

#include "gpio_chip.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── 芯片注册表 ──────────────────────────────────── */

#define MAX_CHIPS	8

/*
 * s_chips[] - 静态数组，保存所有已注册的 gpio_chip 指针。
 *
 * 真实内核中用更复杂的链表 + IDA 分配器（gpiochip_add_data
 * 内部调 gpiochip_add_to_list），这里简化为固定大小数组。
 */
static struct gpio_chip *s_chips[MAX_CHIPS];
static int s_num_chips;

/* ── Provider API ────────────────────────────────── */

int gpiochip_add(struct gpio_chip *gc)
{
	/*
	 * 将 chip 实例加入全局注册表。
	 * 如果注册成功，consumer 就能通过 gpio_get_desc 找到这块芯片。
	 * 注册失败通常意味着芯片数量超限或 gc 无效。
	 */
	if (s_num_chips >= MAX_CHIPS)
		return -1;
	s_chips[s_num_chips++] = gc;
	printf("[gpiolib] chip '%s' registered (base=%u, ngpio=%u)\n",
	       gc->label, gc->base, gc->ngpio);
	return 0;
}

/* ── 查找接口 ────────────────────────────────────── */

struct gpio_desc *gpio_get_desc(const char *chip_label, unsigned int offset)
{
	/*
	 * 遍历全局注册表，按 label 字符串匹配目标芯片。
	 * 找到后动态分配 gpio_desc（调用方 free）。
	 *
	 * 这里用 label 查，实际上是双向绑定：
	 *   desc->gc     = 找到的 chip 实例
	 *   desc->offset = 传入的芯片内部引脚号
	 * 之后所有操作 desc 的调用都能直接通过 desc->gc 定位芯片。
	 *
	 * 真实内核中这个查找通过 device tree 节点 + of_get_named_gpio
	 * 完成，最终也是落到一个 gpio_desc。
	 */
	for (int i = 0; i < s_num_chips; i++) {
		if (strcmp(s_chips[i]->label, chip_label) == 0) {
			struct gpio_desc *d = malloc(sizeof(*d));
			d->gc     = s_chips[i];
			d->offset = offset;
			return d;
		}
	}
	return NULL;
}

/* ── Consumer API ────────────────────────────────── */

void gpiod_set_value(struct gpio_desc *desc, int value)
{
	/*
	 * 空指针保护：内核中也有类似检查（gpiod_put 后 desc 可能无效）
	 */
	if (!desc || !desc->gc)
		return;

	/*
	 * ┌─ 多态 dispatch 的关键一行 ──────────────────┐
	 * │ desc->gc->set(desc->gc, desc->offset, value) │
	 * │                                              │
	 * │ 这一行的执行路径取决于 desc->gc 指向谁：      │
	 * │   → 指向 vendor_a_chip → 走 vendor_a_set     │
	 * │   → 指向 vendor_b_chip → 走 vendor_b_set     │
	 * │                                              │
	 * │ 这是函数指针多态在 C 语言中最纯粹的形式。     │
	 * │ 整个分层架构设计，最终都在为这一行服务。      │
	 * │                                              │
	 * │ 真实内核等价行：include/linux/gpio/driver.h   │
	 * │   gc->set(gc, gpio_chip_hwgpio(desc), value) │
	 * └──────────────────────────────────────────────┘
	 *
	 * ┌─ 调用链全景 ──────────────────────────────────┐
	 * │                                                │
	 * │  main.c         leds_gpio.c     gpiolib.c      │
	 * │    │                │              │           │
	 * │  gpio_get_desc() ──┼──────────────┼─ 遍历查找  │
	 * │    │                │              │ 分配 desc  │
	 * │    │                │              │ desc->gc = │
	 * │    │                │              │ &vendorX   │
	 * │    │                │              │           │
	 * │  led_gpio_brig ──── gpiod_set ──── desc->gc->set()
	 * │  htness_set()     value()       ← 就是这一行   │
	 * │    │                │              │           │
	 * │                     vendor_a_set() / vendor_b_set()
	 * │                       ↑ 多态 dispatch 落点     │
	 * └────────────────────────────────────────────────┘
	 */
	desc->gc->set(desc->gc, desc->offset, value);
}

int gpiod_get_value(struct gpio_desc *desc)
{
	if (!desc || !desc->gc)
		return -1;
	/*
	 * 与 set 对称：通过 desc->gc->get 多态 dispatch。
	 */
	return desc->gc->get(desc->gc, desc->offset);
}
