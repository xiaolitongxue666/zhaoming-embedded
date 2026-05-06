/* SPDX-License-Identifier: MIT */
/**
 * @file  led.h
 * @brief LED 基类 + 三种子类 - 向上转型工程化高潮
 *
 * @details
 * 本章 (ch12) 把前面演化好的 ops 表机制放进真实工程结构:
 *   - 父类 struct led_base: 所有 LED 共有字段 (ops + name + is_on)
 *   - 子类 struct led_gpio / led_pwm / led_i2c: 三种硬件实现,
 *     base 嵌在第一个字段, 后面追加各自的硬件资源 (pin / channel /
 *     bus + addr). 父类放共性、硬件字段下沉到子类.
 *
 * 关键 ABI 不变量 (见 ch12 § 12.2, C99 § 6.7.2.1):
 *   "结构体第一个成员的地址等于结构体本身的地址."
 * 这一条让
 *     struct led_base *handle = &gpio_led.base;
 * 合法 (类型对齐) 又零开销 (偏移 0, 不生成加法指令). 这是整章
 * 所有威力的根基.
 *
 * 应用层入口只有两个函数: led_on / led_off, 都接 struct led_base *
 * 句柄. 应用层一行硬件字样都没有.
 */

#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdbool.h>

struct led_base;

/*
 * struct led_ops - 操作表 (ops 表). 子类把自己的实现填进去,
 * 父类统一接口 led_on / led_off 通过 ops 表分发到子类.
 */
struct led_ops {
	int (*on)(struct led_base *me);
	int (*off)(struct led_base *me);
};

/*
 * struct led_base - 父类.
 *
 * 三个字段:
 *   ops    : 子类的操作表入口, 必须放第一个 (向上转型零开销)
 *   name   : 给日志打印, 也是子类的"我是谁"标识
 *   is_on  : 当前开关状态
 */
struct led_base {
	const struct led_ops *ops;
	const char           *name;
	bool                  is_on;
};

/* 应用层入口: 所有调用都走 led_base 句柄 */
int led_on(struct led_base *me);
int led_off(struct led_base *me);

/*
 * ------- 子类一: GPIO LED -------
 *
 * 最简单的 LED: 一个 GPIO 引脚拉高 (或拉低) 点亮.
 * base 必须在第一个字段, 后面跟硬件资源.
 *
 * on_level 让同一份 led_gpio 子类支持两种接法: 高电平点亮 (LED
 * 阴极接 GPIO, 阳极接 VCC) / 低电平点亮 (反过来, LED 共阳极).
 * 不用为这两种接法各写一个 led_gpio 子类, 用 bool 字段区分就行.
 */
struct led_gpio {
	struct led_base base;       /* 父类, 第 0 字段 (向上转型不变量) */
	uint8_t         pin;
	bool            on_level;   /* 1 = 高电平点亮, 0 = 低电平点亮 */
};

void led_gpio_init(struct led_gpio *me, const char *name,
		   uint8_t pin, bool on_level);

/*
 * ------- 子类二: PWM LED -------
 *
 * 通过 PWM 占空比驱动. 硬件资源: 一路 PWM 通道 + 当前占空比.
 */
struct led_pwm {
	struct led_base base;       /* 父类, 第 0 字段 */
	uint8_t         channel;
	uint8_t         duty;
};

void led_pwm_init(struct led_pwm *me, const char *name,
		  uint8_t channel, uint8_t duty);

/*
 * ------- 子类三: I2C 扩展芯片 LED -------
 *
 * 走 I2C 总线给某个寄存器写 0/1 控制一颗 LED 亮灭. 硬件资源:
 * 总线编号 + 7-bit 设备地址.
 *
 * 这种结构在工业控制板里很常见 (主控 GPIO 不够用, 挂一颗 PCA9555
 * 之类的 I/O 扩展芯片). 应用层完全不知道, 它只调 led_on(handle).
 */
struct led_i2c {
	struct led_base base;       /* 父类, 第 0 字段 */
	uint8_t         bus;
	uint8_t         addr;
};

void led_i2c_init(struct led_i2c *me, const char *name,
		  uint8_t bus, uint8_t addr);

#endif /* LED_H */
