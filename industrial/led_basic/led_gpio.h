/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.h - LED GPIO 子类 (拉线点亮).
 *
 * 基类放第一字段, 上转 / 回查都是直接 cast (基类不在第一字段时才需要
 * container_of). 不实现 set_brightness, 让父类走默认 no-op.
 */

#ifndef __LED_GPIO_H
#define __LED_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#include "led_base.h"
#include "platform_def.h"

struct led_gpio {
	struct led_base base;        /* 基类放第一字段, 上转直接 cast */
	int32_t         pin;         /* platform_pin_get 解析后的 pin 号 */
	bool            active_high; /* true: 高电平点亮 */
};

/* 构造函数.
 *   me           子类实例
 *   name         实例名 (如 "status", "alarm")
 *   pin_name     平台 pin 名 (如 "PI.15")
 *   active_high  true 表示高电平点亮
 */
platform_err_t led_gpio_init(struct led_gpio *me, const char *name,
                             const char *pin_name, bool active_high);

#endif /* __LED_GPIO_H */
