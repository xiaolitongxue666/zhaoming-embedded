/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief 父类层公开头 - 字段集 + ops 表 + 共有 init + 父类统一接口
 *
 * @details
 * 父类层公开头, 跟 ch11 一脉相承的"集中点":
 *   - struct led_base 字段集 (ops + name + is_on)
 *   - struct led_ops  操作表字段集 (本章只用 on / off, ch13 起会扩)
 *   - led_base_init   共有 init, 子类 init 第一行调一次
 *   - led_on/led_off  父类统一接口, 函数体一行 me->ops->xxx(me)
 *
 * 应用层 #include "led_base.h" 一行就拿到全部父类层公开 API. 子类头
 * (led_gpio.h / led_pwm.h / led_i2c.h) 再各自 #include "led_base.h"
 * 拿到这套字段集和接口, 应用层不直接 #include 子类头.
 *
 * ch12 这一章新增 leds.h 做板级聚合 (extern 全局 base 句柄 + led_board_init
 * 声明). 应用层只 #include "leds.h" 就够 -- "应用层只看得到 base 句柄,
 * 看不到 GPIO / PWM / I2C 哪种子类" 是 ch12 § 12.4 / § 12.10 主线.
 *
 * ABI 不变量 (见 ch12 § 12.2, C99 § 6.7.2.1):
 *   "结构体第一个成员的地址等于结构体本身的地址."
 * 这一条让 &gpio_led.base 合法 (类型对齐) 又零开销 (偏移 0, 不生成
 * 加法指令), 是整章所有威力的根基.
 *
 * 本章 (ch12) NULL 处理走"返回 -1 错误码", 还没引入 assert 必填策略.
 * 必填 + 选填两种策略 + sensor 接口风格的全必填策略, ch14 § 14.2 / § 14.3
 * / § 14.4 三节集中展开.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include <stdint.h>
#include <stdbool.h>

struct led_base;

/*
 * struct led_ops - 操作表 (ops 表). 子类把自己的实现填进去,
 * 父类统一接口 led_on / led_off 通过 ops 表分发到子类.
 *
 * ch12 主题是向上转型, 应用层只用得着 on / off, 表先收缩成两字段.
 * ch13 起按需要再加字段 (set_brightness 等).
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
 *   name   : 给日志打印, 也是子类的 "我是谁" 标识
 *   is_on  : 当前开关状态
 */
struct led_base {
	const struct led_ops *ops;     /* 第一个字段, 对象起始地址处 */
	const char           *name;
	bool                  is_on;
};

int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);

/*
 * 父类统一接口 - 实现在 led_base.c, 所有子类共用.
 * 应用层只调 led_on / led_off, 看不到 ops 字段.
 */
int led_on(struct led_base *me);
int led_off(struct led_base *me);

#endif /* LED_BASE_H */
