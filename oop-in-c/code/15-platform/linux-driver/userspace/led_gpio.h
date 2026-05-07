/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.h - 子类一: GPIO LED (ch15 linux-driver/userspace 版).
 *
 * 直接持有 libgpiod 1.x 的 struct gpiod_line *, 不再走 platform 抽象层.
 * 内核 gpiolib + chardev 已经把 "选哪个 GPIO 控制器 / 申请哪条 line" 抽得
 * 干干净净, 应用层再套一层 platform_gpio_init 就是过度封装.
 */

#ifndef LED_GPIO_H
#define LED_GPIO_H

#include "led_base.h"

struct gpiod_chip;
struct gpiod_line;

struct led_gpio {
	struct led_base    base;        /* 父类, 第 0 字段 */
	struct gpiod_line *line;        /* libgpiod 申请到的 line 句柄 */
	bool               active_high; /* 1 = 高电平点亮, 0 = 低电平点亮 */
};

int led_gpio_init(struct led_gpio *me, const char *name,
                  struct gpiod_chip *chip, unsigned int line_offset,
                  bool active_high);
void led_gpio_deinit(struct led_gpio *me);

#endif /* LED_GPIO_H */
