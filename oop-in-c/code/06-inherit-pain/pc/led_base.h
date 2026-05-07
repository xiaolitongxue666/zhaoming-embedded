/* SPDX-License-Identifier: MIT */
/*
 * led_base.h - LED 公共基类抽象
 *
 * 见书 ch06 § 6.3 提公因式 + § 6.6 这个东西叫继承。
 *
 * 八种 LED 都有 name + is_on 这两个共有字段。把它们提到一个独立
 * 的 struct led_base 里, 每种 LED 子类把 led_base 嵌套进自己的
 * struct 第一个位置 -- 这就是继承的 C 语言写法。
 *
 * 数学课上老师讲过: 3a + 3b + 3c = 3(a + b + c)。前面那个 3
 * 重复三次就提到括号外面。代码里一模一样, name + is_on 在八种
 * LED 里抄八遍, 那就提到 base 里写一份。
 *
 * "公共部分放第一个位置" 这一章只关心 "公共字段写一次就够"
 * 这个直接收益。
 *
 * 注意: base 里没有 pin。pin 是 GPIO 子类特有的硬件参数,
 * 在 led_gpio.h 里。PWM 子类有自己的 channel/duty, I2C 子类有
 * 自己的 bus/addr。基类只放所有 LED 都共有的状态层信息 ——
 * 把硬件特定字段下沉到子类, 每个 base 字段都是这一类设备真正
 * 共有的, 不会出现 "GPIO 没有 channel" 这种放错地方的字段。
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

struct led_base {
	const char *name;       /* 给日志打印用，例如 "red" */
	bool        is_on;      /* 当前开关状态 */
};

int led_base_init(struct led_base *me, const char *name);

const char *led_base_get_name(const struct led_base *me);
bool        led_base_is_on(const struct led_base *me);

#endif /* LED_BASE_H */
