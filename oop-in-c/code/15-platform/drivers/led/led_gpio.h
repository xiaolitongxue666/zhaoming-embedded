/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.h - LED GPIO 子类 (拉线点亮).
 *
 * 基类放第一字段, 上转 / 回查都是直接 cast (基类不在第一字段时才需要
 * container_of). 不实现 set_brightness, 走父类默认 no-op.
 *
 * 子类底下只调 platform_pin_xxx ops 表层接口, 永远不直接碰
 * GPIO_TypeDef 寄存器. 跨 MCU 移植时这一份代码 0 改动, 只换
 * platform/arch/<mcu>/pin_board.c 一份.
 */

#ifndef DRIVERS_LED_LED_GPIO_H
#define DRIVERS_LED_LED_GPIO_H

#include "drivers/led/led_base.h"

struct led_gpio {
	struct led_base base;        /* 基类放第一字段, 上转直接 cast */
	int32_t         pin;         /* platform_pin_get 解析后的 pin 号 */
	bool            active_high; /* true: 高电平点亮 */
};

/* 构造函数.
 *   me           子类实例
 *   name         实例名 (如 "ERR", "STAT")
 *   pin_name     平台 pin 名 (如 "PD.12", 由 platform_pin_get 解析)
 *   active_high  true 表示高电平点亮
 */
int led_gpio_init(struct led_gpio *me, const char *name,
                  const char *pin_name, bool active_high);

#endif /* DRIVERS_LED_LED_GPIO_H */
