/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.h - GPIO LED 子类 (ch14 版, 选填策略示例)
 *
 * GPIO 子类只填 on / off, 故意不填 set_brightness, 演示 ch14 § 14.3
 * 的选填策略 -- 字段为 NULL, 父类的统一接口走默认 "安静跳过".
 *
 * GPIO 灯没有亮度概念, 不该让它写一个空的 gpio_set_brightness 函数.
 * 把 NULL 这一情况移到父类统一接口里集中处理, ops 表本身不动.
 */

#ifndef LED_GPIO_H
#define LED_GPIO_H

#include "led.h"

/* GPIO 子类: 只填 on / off, 不支持调光 */
struct led_gpio {
	struct led_base base;
	uint8_t         pin;
	bool            on_level;
};

int led_gpio_init(struct led_gpio *me, const char *name,
                  uint8_t pin, bool on_level);


#endif /* LED_GPIO_H */
