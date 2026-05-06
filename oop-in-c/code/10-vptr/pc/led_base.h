/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief vptr 落地: ops 字段塞进 led_base 第一个位置
 *
 * @details
 * 本章核心问题 (见 ch10 § 10.1):
 *   ch09 把多个函数指针打包成 struct led_ops 一张表, 但应用层每次
 *   调用都得自己传 ops:
 *       test_led(&red_led.base, &led_ops_gpio);
 *       test_led(&blue_led.base, &led_ops_pwm);
 *   应用层得记住"红灯用 gpio 表、蓝灯用 pwm 表". 一旦传错, 又是 bug.
 *
 * 答案: 让每颗 LED 自己带着自己的 ops 表. struct led_base 加一个
 * const struct led_ops *ops 字段, init 时填进去, 调用时从字段取.
 *
 *   struct led_base {
 *       const struct led_ops *ops;   <-- 新增, 必须放第一个字段
 *       const char *name;
 *       bool        is_on;
 *   };
 *
 * 为什么 ops 放第一个 (见 ch10 § 10.3, § 10.8.1.1, § 10.8.1.2):
 *   1) 向上转型零开销 - &red_led 和 &red_led.base 是同一个地址
 *      (C99 § 6.7.2.1: "结构体第一个成员的地址等于结构体本身的
 *       地址"), 编译期不生成任何加法指令
 *   2) 取 vptr 是单条 LDR - me->ops 编译成 LDR r3, [r0] (不带偏移),
 *      ARM Cortex-M4 上一条指令一个周期 + 一次访存
 *   3) cacheline 友好 - 对象起始处必然落 cacheline 头部, 把最常
 *      访问的 ops 字段放头部, 命中率最高
 *
 * 这就是 C++ 编译器看到 class { virtual ... } 时偷偷做的事: vptr
 * 放在对象的最前面 (GCC / Clang / MSVC 三大编译器单继承一致).
 * Linux 内核 / Zephyr / GObject 都遵守这个约定. 这是世界标准.
 *
 * "为什么 ops 不放进子类 (struct led_gpio) 而要放进 base" 见
 * ch10 § 10.8.5: 放进 base 让基类层 dispatch 函数 led_on(base) 能
 * 直接 me->ops->on(me) 调用, 不用关心是哪种子类. 加新 LED 类型
 * 不改 led_on. 这就是 Linux 内核 file->f_op->read(file) 的机制.
 *
 * 为什么字段类型是 "const struct led_ops *" 而不是 "struct led_ops
 * * const" (见 ch10 § 10.8.2):
 *   const 修饰 led_ops, 意思是"指向常量 led_ops 的指针". 表本身
 *   不允许改 (me->ops->on = ... 编译报错), 但指针可以重新指向另一
 *   张 ops 表 (me->ops = ... 合法 -- 极端场景下用于运行时换实现).
 *   工业代码 99% 用这种风格, Linux 内核 file_operations 也是.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

/*
 * 前向声明 - led_ops 完整定义在 led.h.
 * 这里只用到指针类型, 编译器只需要知道"led_ops 是个 struct",
 * 不需要看到字段集. 这样 led_base.h 不依赖 led.h, 减少头文件
 * 耦合 -- 哪天 led_ops 字段改了, 不会触发 led_base.h 重新编译.
 */
struct led_ops;

struct led_base {
	const struct led_ops *ops;     /* vptr 落地, 必须是第一个字段 */
	const char           *name;
	bool                  is_on;
};

int led_base_init(struct led_base *me, const char *name,
                  const struct led_ops *ops);
const char *led_base_get_name(const struct led_base *me);

#endif /* LED_BASE_H */
