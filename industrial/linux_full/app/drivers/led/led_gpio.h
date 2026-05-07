/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.h - LED GPIO 子类 (基于 libgpiod 实现).
 *
 * 这一份子类直接调 libgpiod, 不再经过任何 platform 抽象层 -- Linux 内核
 * 已经把 GPIO platform 抽象做完了 (gpio chardev / gpiolib), libgpiod 是
 * 内核暴露给用户态的 OOP 接口, 用户态再套一层是过度封装.
 *
 * 基类放第一字段, 上转 / 回查都是直接 cast (基类不在第一字段时才需要
 * container_of). 不实现 set_brightness, 让父类走默认 no-op.
 */

#ifndef DRIVERS_LED_LED_GPIO_H_
#define DRIVERS_LED_LED_GPIO_H_

#include <stdbool.h>
#include <stdint.h>

#include "drivers/led/led_base.h"
#include "led_errors.h"

/* 前向声明: 不让 led_gpio.h 把 <gpiod.h> 拖给所有 includer.
 * 应用层只见 struct led_base *, 不会触到这个前向声明; 装配代码
 * (environment_cfg/led_cfg.c) 才需要 #include <gpiod.h>. */
struct gpiod_chip;
struct gpiod_line;

struct led_gpio {
	struct led_base    base;        /* 基类放第一字段, 上转直接 cast */
	struct gpiod_line *line;        /* 由 gpiod_chip_get_line 拿到 */
	bool               active_high; /* true: 高电平点亮 */
};

/* 构造函数.
 *   me           子类实例
 *   name         实例名 (如 "status", "alarm")
 *   chip         libgpiod chip 句柄 (装配代码 gpiod_chip_open_by_name 拿到)
 *   line_offset  BCM 编号 (树莓派 4B 上 GPIO17 -> 17, GPIO22 -> 22)
 *   active_high  true 表示高电平点亮
 *
 * 内部调用顺序:
 *   1. gpiod_chip_get_line(chip, line_offset)
 *   2. gpiod_line_request_output(line, "led", initial_value)
 *   3. led_base_init 填 ops 表
 *
 * 失败时返回 PLATFORM_EIO (libgpiod 调用失败) 或 PLATFORM_EINVAL (参数非法).
 */
platform_err_t led_gpio_init(struct led_gpio *me, const char *name,
                             struct gpiod_chip *chip,
                             unsigned int line_offset, bool active_high);

/* 释放 line. led_gpio 实例本身是静态分配的, 这里只 release line. */
void led_gpio_deinit(struct led_gpio *me);

#endif /* DRIVERS_LED_LED_GPIO_H_ */
