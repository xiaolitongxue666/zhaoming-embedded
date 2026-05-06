/* SPDX-License-Identifier: MIT */
/**
 * @file  led.h
 * @brief 引入 struct led_ops - 多个函数指针打包成一张操作接口表
 *
 * @details
 * 本章核心问题 (见 ch09 § 9.1):
 *   ch08 的 test_led 接受三个独立函数指针参数:
 *       int test_led(struct led_base *me,
 *                    int (*on)(struct led_base *),
 *                    int (*off)(struct led_base *),
 *                    int (*toggle)(struct led_base *));
 *   再加 set_brightness、get_state, 参数列表能长到换行. 更要命:
 *   on / off / toggle 类型完全一样, 调用方手抖把 on 和 off 顺序传反,
 *   编译器看不出来 -- 该亮的时候灭, 该灭的时候亮, 调一天.
 *
 * 答案: 把一组绑死的函数指针打包进一个 struct, 按名字访问.
 *   - 参数从 4 个塞回 1 个 (ops 指针)
 *   - ops->on 永远是 on, 不可能传反 (编译器在 designated initializer
 *     里就把字段类型对上了)
 *   - 加新行为 (set_brightness) 在 ops 里加字段, 老调用方一字不改
 *
 * 这就是 Linux 内核 struct file_operations / net_device_ops 的骨架.
 * C++ 编译器看到带 virtual 的 class 也是偷偷生成这张表 (vtable),
 * 你这一章手动做一遍. 见 ch09 § 9.4 (C 对比 C++).
 *
 * 子类 struct led_gpio / led_pwm 这一章故意保持 ch07/ch08 的字段集
 * (base + 硬件参数), 不在子类里挂 ops 字段. ops 字段什么时候放进
 * 哪个 struct, 是 ch10 § 10.3 的核心决策 -- "放进 led_base 而不是
 * 子类", 一处定义、全员共享.
 *
 * test_led(me, ops) 这一章只接受外部传 ops 指针. ch10 起 ops 直接
 * 从 me 自己身上拿 (me->ops), 应用层连 ops 都不用管.
 */

#ifndef LED_H
#define LED_H

#include "led_base.h"

/*
 * led_action_fn - 用 typedef 给函数指针类型起短名.
 *
 * "int (*)(struct led_base *)" 写一次还行, ops 表里写三次就累.
 * Linus 反对 typedef struct (藏类型信息), 但函数指针 typedef 是
 * 他公开支持的例外: 类型字面量太长, 起短名纯收益.
 * 见 ch09 § 9.2.
 */
typedef int (*led_action_fn)(struct led_base *me);

/*
 * struct led_ops - 操作表 (ops table, 也叫 vtable / virtual table).
 *
 * 把同一种 LED 的所有"可变行为"打包. 一种 LED 实现填一张表,
 * test_led 内部按名字 ops->on / ops->off 访问, 再也不会传反.
 *
 * 按名访问还有一个好处: 加新行为 (set_brightness) 只在 ops 里加
 * 字段, 老的 on/off 调用一字不改 (designated initializer 把
 * 没列出的字段默认填 NULL, 见 ch09 § 9.5.1).
 */
struct led_ops {
	led_action_fn on;
	led_action_fn off;
	led_action_fn toggle;
};

/* GPIO 子类: pin 在子类里 */
struct led_gpio {
	struct led_base base;
	uint8_t         pin;
};

/* PWM 子类: channel + duty 在子类里 */
struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};

int led_gpio_init(struct led_gpio *me, const char *name, uint8_t pin);
int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty);

/*
 * test_led 现在接 ops 指针, 一个参数搞定三件事 (on/off/toggle).
 * me 一个, ops 一个, 按名访问不会再传反.
 *
 * 第二个参数声明成 const struct led_ops * 而不是 struct led_ops *,
 * 表示 test_led 不会改 ops 表内容. 这一层 const 把"我手里这个 ops
 * 表别动"的意图传递到调用边界. 见 ch08 § 8.6.5 / ch10 § 10.8.2:
 * "const 修饰它右边的 token" -- 这里 const 在 struct led_ops 左边
 * 时, 等价于在右边 (C 语法两种写法都允许), 修饰的是 ops 指向的内容.
 */
int test_led(struct led_base *me, const struct led_ops *ops);

/*
 * 两张现成的 ops 表 - extern 声明, 定义在 led.c.
 *
 * const + extern 的组合让这两张表落进 .rodata 段:
 *   - MCU 上烧到 Flash, 完全不占 RAM
 *   - 100 颗同类型 LED 共享同一张 12 字节的表 (见 ch09 § 9.5.3)
 *   - 链接期不可改, 防止运行时把 .on 字段意外改成野指针
 */
extern const struct led_ops led_ops_gpio;
extern const struct led_ops led_ops_pwm;

#endif /* LED_H */
