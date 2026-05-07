/* SPDX-License-Identifier: MIT */
/*
 * led_base.h - 父类层接口 (ch15 完整版, 风格 A)
 *
 * 这一份头跟 ch10 / ch11 一脉相承: 父类只关心共有数据 (ops + name +
 * is_on) 和共有 init. 子类的 init 第一行调 led_base_init 把对应的
 * const ops 表交给父类一次填好, 应用层只对 struct led_base * 句柄
 * 编程, 看不到子类长什么样.
 *
 * ch15 在 ch11 父类层的基础上做两件事:
 *   1) ops 表新增第三个槽位 set_brightness, 采用 ch14 的"必填 / 选填"
 *      混合策略 (on / off 必填, set_brightness 选填)
 *   2) 父类统一接口 led_set_brightness 在选填字段为 NULL 时走父类的
 *      默认行为 (打印一行 "no dimming, skip"), 不崩
 *
 * 见 ch15 § 15.2 父类层 + ch11 § 11.4 父类统一接口 + ch14 § 14.2 / § 14.3.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

/*
 * 前向声明 - led_ops 完整定义在 led.h.
 * 这里只用到指针类型, 编译器只需要知道"led_ops 是个 struct",
 * 不需要看到字段集. 这样 led_base.h 不依赖 led.h, 减少头文件
 * 耦合 -- 哪天 led_ops 字段改了, 不会触发 led_base.h 重新编译.
 * 同 ch10 / ch11 风格.
 */
struct led_ops;

struct led_base {
	const struct led_ops *ops;     /* 第一个字段, 对象起始地址处 */
	const char           *name;
	bool                  is_on;
};

/*
 * led_base_init - 父类共有 init.
 *
 * 子类 init (led_gpio_init / led_pwm_init / led_i2c_init) 第一行调本
 * 函数, 把"我用哪张 ops 表"作为 const 参数传进来. 父类把 ops / name
 * 一次性存到 me 字段, 顺手把 is_on 置成 false (上电默认灭).
 *
 * 返回 0 表示成功, -1 表示参数非法 (任意一个指针为 NULL). 子类 init
 * 拿到返回值要往外传, board_init 那一层才能在出错时立刻知道哪盏灯
 * 没初始化好.
 */
int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);

#endif /* LED_BASE_H */
