/* SPDX-License-Identifier: MIT */
/*
 * led_base.h - 父类层接口 + ops 表 (ch15 完整版, 风格 A)
 *
 * 这一份头跟 ch10 / ch11 / ch12 一脉相承: 父类只关心共有数据 (ops + name +
 * is_on) 和共有 init. 子类的 init 第一行调 led_base_init 把对应的 const
 * ops 表交给父类一次填好, 应用层只对 struct led_base * 句柄编程, 看不到
 * 子类长什么样.
 *
 * ch15 在 ch11 父类层的基础上做两件事:
 *   1) ops 表新增第三个槽位 set_brightness, 采用 ch14 的 "必填 / 选填"
 *      混合策略 (on / off 必填, set_brightness 选填)
 *   2) 父类统一接口 led_set_brightness 在选填字段为 NULL 时走父类的
 *      默认行为 (打印一行 "no dimming, skip"), 不崩
 *
 * struct led_ops 的完整定义放在这里. 子类头 (led_gpio.h / led_pwm.h /
 * led_i2c.h) 只 #include "led_base.h" 就够拿到 ops 表. 这样 led_base.c
 * 的 dispatch 函数可以直接看到 ops 字段集, 不再需要前向声明.
 *
 * 见 ch15 § 15.2 父类层 + ch11 § 11.4 父类统一接口 + ch14 § 14.2 / § 14.3.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

struct led_base;

/*
 * struct led_ops - 操作表 (ch14 的 "必填 + 选填" 混合形态).
 *
 * on / off 必填: 子类没填, 父类统一接口里的 assert 立刻报错.
 * set_brightness 选填: 子类没填, 父类统一接口走默认行为 (打印一行
 * "no dimming, skip"), 不崩.
 *
 * 这两种语义对应 C++ 的纯虚函数 (virtual void f() = 0;) 和带默认行为
 * 的虚函数 (virtual void f() { 默认实现 }), 见 ch14 § 14.2 / § 14.3.
 */
struct led_ops {
	int (*on)(struct led_base *me);                 /* 必填 */
	int (*off)(struct led_base *me);                /* 必填 */
	int (*set_brightness)(struct led_base *me,      /* 选填 */
	                      uint8_t brightness);
};

struct led_base {
	const struct led_ops *ops;     /* 第一个字段, 对象起始地址处 */
	const char           *name;
	bool                  is_on;
};

/*
 * led_base_init - 父类共有 init.
 *
 * 子类 init (led_gpio_init / led_pwm_init / led_i2c_init) 第一行调本
 * 函数, 把 "我用哪张 ops 表" 作为 const 参数传进来. 父类把 ops / name
 * 一次性存到 me 字段, 顺手把 is_on 置成 false (上电默认灭).
 *
 * 返回 0 表示成功, -1 表示参数非法 (任意一个指针为 NULL). 子类 init
 * 拿到返回值要往外传, led_board_init 那一层才能在出错时立刻知道哪盏灯
 * 没初始化好.
 */
int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);

/*
 * 父类统一接口 - 写在 led_base.c, 所有子类共用.
 * 应用层只调 led_on / led_off / led_set_brightness, 看不到 ops 字段.
 */
int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_set_brightness(struct led_base *me, uint8_t brightness);

#endif /* LED_BASE_H */
