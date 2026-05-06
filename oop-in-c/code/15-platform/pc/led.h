/* SPDX-License-Identifier: MIT */
/*
 * led.h - LED 驱动层 (ch15 完整版)
 *
 * 这一层把 ch07 - ch14 学过的所有 OOP 武器组装在一起:
 *   - struct led_base + ops 表 (ch11 多态完整图景)
 *   - 子类把 base 嵌进去做"继承" (ch06)
 *   - 父类统一接口走 ops dispatch (ch11)
 *   - 必填 / 选填混用 (ch14: on/off 必填, set_brightness 选填)
 *   - 子类实现里 container_of 反推 (ch13)
 *   - 子类调 platform_gpio_xxx 封装函数, 从来不碰寄存器 (ch01 - ch10)
 *
 * 应用层只用 led_base 句柄, 分不清底下是 GPIO / PWM / I2C.
 *
 * 见 ch15 § 15.2 父类层 + § 15.3 子类层.
 */

#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdbool.h>

struct led_base;

struct led_ops {
	int (*on)(struct led_base *me);                 /* 必填 */
	int (*off)(struct led_base *me);                /* 必填 */
	int (*set_brightness)(struct led_base *me,      /* 选填 */
			      uint8_t brightness);
};

struct led_base {
	const struct led_ops *ops;
	const char           *name;
	bool                  is_on;
};

int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_set_brightness(struct led_base *me, uint8_t brightness);

/* GPIO 子类 */
struct led_gpio {
	struct led_base base;
	uint8_t         pin;
	bool            on_level;
};
void led_gpio_init(struct led_gpio *me, const char *name,
		   uint8_t pin, bool on_level);

/* PWM 子类 */
struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};
void led_pwm_init(struct led_pwm *me, const char *name,
		  uint8_t channel, uint8_t duty);

/* I2C 子类 */
struct led_i2c {
	struct led_base base;
	uint8_t         bus;
	uint8_t         addr;
};
void led_i2c_init(struct led_i2c *me, const char *name,
		  uint8_t bus, uint8_t addr);

#endif /* LED_H */
