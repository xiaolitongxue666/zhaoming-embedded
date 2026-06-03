/* SPDX-License-Identifier: MIT */
/*
 * leds_gpio.c - LED GPIO 驱动（设备驱动层）
 *
 * ┌─ 第4层视角：设备驱动作者的世界 ────────────────────┐
 * │ 这个文件代表"设备驱动工程师"写的代码。              │
 * │ 他们的工作：调 gpiod_set_value 控制 LED 亮灭。      │
 * │ 他们不知道也不需要知道：                             │
 * │   - 运行的 SoC 是哪家的                             │
 * │   - GPIO 寄存器的布局（DR_REG 还是 BSRR）            │
 * │   - 芯片提供了多少个引脚                             │
 * │   - 当前 GPIO 要不要先 request / direction_output    │
 * │                                                        │
 * │ 是底层的 gpiolib + gpio_chip 把所有这些差异屏蔽了。    │
 * │ 这就是"分层抽象"给驱动作者的最大礼物。                 │
 * │                                                        │
 * │ 真实内核版：drivers/leds/leds-gpio.c                   │
 * │ 参见 ch16 §16.4 乘法变加法                             │
 * └────────────────────────────────────────────────────────┘
 *
 * ┌─ N × M → N + M 的 ASCII 图解 ─────────────────────┐
 * │                                                        │
 * │  　             LED 驱动   按键驱动    LCD 驱动          │
 * │                     │          │          │             │
 * │  ┌──────────────────┼──────────┼──────────┼──────────┐ │
 * │  │          gpiolib 抽象层 (gpio_chip + gpiod_*)     │ │
 * │  └──────────────────┼──────────┼──────────┼──────────┘ │
 * │                     │          │          │             │
 * │               ┌─────┴────┐ ┌──┴───┐ ┌───┴────┐        │
 * │               │ vendorA  │ │ vendorB │ │ vendorC │        │
 * │               │ (片内)    │ │ (外扩)   │ │ (I2C)   │        │
 * │               └──────────┘ └────────┘ └─────────┘        │
 * │                                                        │
 * │  没有抽象层：3 个设备驱动 × 3 家芯片 = 9 份驱动代码     │
 * │  有抽象层：  3 个设备驱动 + 3 家芯片驱动 = 6 份代码     │
 * │  芯片越多、设备越多，这个差值越大。                      │
 * │  这就是分层架构的核心收益。                              │
 * └────────────────────────────────────────────────────────┘
 */

#include "gpio_chip.h"
#include <stdio.h>

/**
 * led_gpio_brightness_set - 设置 LED 亮度（实质是 GPIO 电平）
 * @desc:  LED 对应的 GPIO 引脚描述符
 * @value: 亮度值（0 = 灭，非 0 = 亮）
 *
 * 内部调 gpiod_set_value，而 gpiod_set_value 内部走
 * desc->gc->set 多态 dispatch，所以：
 *
 *   led_red   (desc->gc = &vendor_a_chip) → vendor_a_set
 *   led_green (desc->gc = &vendor_b_chip) → vendor_b_set
 *
 * 同一个 led_gpio_brightness_set 函数，处理两个不同芯片
 * 的引脚，走的是完全不同的寄存器操作路径。
 * 这就是"代码一次编写，跑遍所有芯片"的底层机制。
 */
void led_gpio_brightness_set(struct gpio_desc *desc, int value)
{
	gpiod_set_value(desc, value);
}
