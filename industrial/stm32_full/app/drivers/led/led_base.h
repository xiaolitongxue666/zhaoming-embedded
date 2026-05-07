/* SPDX-License-Identifier: MIT */
/*
 * led_base.h - LED 基类 + 公开接口.
 *
 * 应用层只见 struct led_base * 句柄, 永远不知道是 GPIO LED / PWM LED /
 * I2C LED. 切换实现把 led_xxx_init 替换即可, 应用层一字不动.
 *
 * 所有 LED 操作 (led_on / led_off / led_set_brightness) 都在这里声明,
 * 应用层只 #include "drivers/led/led_base.h" 就够了, 不要直接 include
 * 子类头文件.
 */

#ifndef DRIVERS_LED_LED_BASE_H_
#define DRIVERS_LED_LED_BASE_H_

#include <stdbool.h>
#include <stdint.h>

#include "platform/platform_def.h"

struct led_base;

/* led_ops - LED 子类必须实现的虚函数表.
 *
 *   on / off                纯虚, 子类必填, 父类 dispatch 时 assert
 *   set_brightness          可选, 子类不填走父类默认 no-op
 */
struct led_ops {
	platform_err_t (*on)(struct led_base *me);
	platform_err_t (*off)(struct led_base *me);
	platform_err_t (*set_brightness)(struct led_base *me, uint8_t level);
};

/* led_base - 所有 LED 子类的父类.
 *
 *   ops    指向子类的虚函数表 (static const, 实例间共享)
 *   name   实例名, 调试 / 日志友好
 *   is_on  当前开关状态, 父类记录, 子类不要直接改
 */
struct led_base {
	const struct led_ops *ops;
	const char           *name;
	bool                  is_on;
};

/* 公开接口 - 应用层只调这一组, 不直接访问 ops */
platform_err_t led_base_init(struct led_base *me, const char *name,
                             const struct led_ops *ops);
platform_err_t led_on(struct led_base *me);
platform_err_t led_off(struct led_base *me);
platform_err_t led_set_brightness(struct led_base *me, uint8_t level);

#endif /* DRIVERS_LED_LED_BASE_H_ */
