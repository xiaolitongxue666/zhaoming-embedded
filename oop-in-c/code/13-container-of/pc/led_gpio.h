/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.h - GPIO LED 子类 (ch13 教学变形版)
 *
 * GPIO 子类故意把 base 挪到第二个字段, 前面塞一个 magic 校验字段, 让
 * offsetof(struct led_gpio, base) != 0. 这正是 ch12 那招 (struct led_gpio *)me
 * 强转会算错地址的布局, 也是 container_of 与位置无关的核心 demo.
 *
 * 真实工业代码 base 仍然推荐放第一个位置 (向上转型零开销, 取 ops 表
 * 也是单条 LDR 指令). 这里挪一下纯粹是为了让 container_of 的位置无关
 * 性肉眼可见. 见 ch13 § 13.8.1 编译器算偏移 + § 13.8.2 位置无关 demo.
 */

#ifndef LED_GPIO_H
#define LED_GPIO_H

#include "led.h"

/*
 * GPIO 子类: base 故意不放第一个位置 (教学变形, 仅本章演示).
 *
 * magic 字段挡在 base 前面, 让 offsetof(struct led_gpio, base) != 0.
 * struct led_base 第一个成员是指针 (4 字节对齐), 编译器会在 magic
 * 后面塞 2 字节 padding, base 落到偏移 4. 运行时 demo 会打印
 * "offsetof(...,base) = 4" 验证.
 */
struct led_gpio {
	uint16_t        magic;     /* 故意挡在前面, 让 base 偏移不为 0 */
	struct led_base base;
	uint8_t         pin;
	bool            on_level;
};

int led_gpio_init(struct led_gpio *me, const char *name,
		  uint8_t pin, bool on_level);

extern const struct led_ops led_ops_gpio;

#endif /* LED_GPIO_H */
