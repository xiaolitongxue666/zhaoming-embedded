/* SPDX-License-Identifier: MIT */
/*
 * led_base.h - LED 基类 + 公开接口 (Linux 用户态版本).
 *
 * 应用层只见 struct led_base * 句柄, 永远不知道是 GPIO LED / PWM LED /
 * I2C LED. 切换实现把 led_xxx_init 替换即可, 应用层一字不动.
 *
 * 跟 stm32_full 的 led_base.h 接口字节级一致 (同一组函数名 / 同一组返回类型 /
 * 同一组 ops 字段), 只是底下不再依赖 platform/ 抽象层 -- 子类直接调
 * libgpiod / sysfs PWM / i2c-dev. 这是这本书工程判断力教学的关键节点:
 * Linux 内核已经把 platform 抽象做完了, 用户态再套一层是过度封装.
 */

#ifndef DRIVERS_LED_LED_BASE_H_
#define DRIVERS_LED_LED_BASE_H_

#include <stdbool.h>
#include <stdint.h>

#include "led_errors.h"

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
