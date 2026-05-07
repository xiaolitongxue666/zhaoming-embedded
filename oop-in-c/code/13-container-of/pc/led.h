/* SPDX-License-Identifier: MIT */
/*
 * led.h - led_ops + 子类 + 父类统一接口 (ch13 版)
 *
 * 跟 ch10/ch11/ch12 一样: 父类 led_base 在 led_base.h, 子类 init
 * 第一行调 led_base_init 把 ops 表交给 base. 这一章 (ch13) 唯一不同
 * 在子类实现层 (led.c): 不再用 (struct led_xxx *)me 强转, 而是用
 * container_of 反推自己.
 *
 * 教学变形: GPIO 子类故意把 base 挪到第二个字段, 前面塞一个 magic
 * 校验字段, 让 offsetof(struct led_gpio, base) != 0. 这正是 ch12 那招
 * 强转会算错地址的布局, 也是 container_of 与位置无关的核心 demo.
 * 真实工业代码 base 仍然推荐放第一个位置 (向上转型零开销, 取 ops
 * 表也是单条 LDR 指令), 这里挪一下纯粹是为了让 container_of 的位置
 * 无关性肉眼可见.
 *
 * led_ops 字段集比 ch12 多一个 set_brightness, PWM 子类填了, GPIO 和
 * I2C 子类没填. 这一章主线是 container_of, set_brightness 字段只是
 * 顺便挂上, 跨章演化更平滑. "字段没填怎么办" 留给 ch14 主题展开.
 *
 * 见 ch13 § 13.6 在 gpio_on 里用一下 + § 13.8.2 base 想放哪就放哪
 *   + § 13.8.6 配套代码 ops 表多了一个字段.
 */

#ifndef LED_H
#define LED_H

#include "led_base.h"
#include <stdint.h>
#include <stdbool.h>

/*
 * struct led_ops - 操作表 (ops 表).
 *
 * 比 ch12 多一个 set_brightness 字段. PWM 子类填了 pwm_set_brightness,
 * GPIO / I2C 子类不填, 字段保持 NULL. led_set_brightness 默认行为是
 * "字段为 NULL 安静返回 0", 跟 ch14 三种策略里"安静默认"那一种对应,
 * 但 ch14 才正式展开"虚函数不实现"的机制讨论.
 */
struct led_ops {
	int (*on)(struct led_base *me);
	int (*off)(struct led_base *me);
	int (*set_brightness)(struct led_base *me, uint8_t brightness);
};

/* 父类统一接口 - 实现在 led.c (调度方有限错误检查 + 走 ops 表) */
int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_set_brightness(struct led_base *me, uint8_t brightness);

/*
 * GPIO 子类: base 故意不放第一个位置 (教学变形, 仅本章演示).
 *
 * magic 字段挡在 base 前面, 让 offsetof(struct led_gpio, base) != 0.
 * struct led_base 第一个成员是指针 (4 字节对齐), 编译器会在 magic
 * 后面塞 2 字节 padding, base 落到偏移 4. 运行时 demo 会打印
 * "offsetof(...,base) = 4" 验证.
 *
 * 真实工业代码里 base 仍推荐放第一个字段. 这里挪一下是为了让
 * container_of "与位置无关" 这一性质能被肉眼看到 -- ch12 那招
 * (struct led_gpio *)me 强转在这种布局下会算错 4 字节地址,
 * container_of 一字不改还是对.
 *
 * 详见 ch13 § 13.8.1 编译器算偏移 + § 13.8.2 位置无关 demo.
 */
struct led_gpio {
	uint16_t        magic;     /* 故意挡在前面, 让 base 偏移不为 0 */
	struct led_base base;
	uint8_t         pin;
	bool            on_level;
};

int led_gpio_init(struct led_gpio *me, const char *name,
		  uint8_t pin, bool on_level);

/* PWM 子类 */
struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};

int led_pwm_init(struct led_pwm *me, const char *name,
		 uint8_t channel, uint8_t duty);

/* I2C 子类 */
struct led_i2c {
	struct led_base base;
	uint8_t         bus;
	uint8_t         addr;
};

int led_i2c_init(struct led_i2c *me, const char *name,
		 uint8_t bus, uint8_t addr);

#endif /* LED_H */
