/* SPDX-License-Identifier: MIT */
/*
 * led.h - led_base + 子类（ch13 版）
 *
 * 和 ch12 唯一的差别：子类实现里用 container_of 反推自己。
 * base 不一定要在第一个位置，本章故意把 GPIO 子类的 base 挪到中间，
 * 证明 container_of 与位置无关——ch12 那种 (struct led_gpio *)me 强转
 * 在这种布局下会算错地址 4 字节，container_of 一直对。
 *
 * 见 ch13 § 13.5 在 gpio_on 里用一下 + § 13.6 base 想放哪就放哪。
 */

#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdbool.h>

struct led_base;

struct led_ops {
	int (*on)(struct led_base *me);
	int (*off)(struct led_base *me);
	int (*set_brightness)(struct led_base *me, uint8_t brightness);
};

struct led_base {
	const struct led_ops *ops;
	const char           *name;
	bool                  is_on;
};

int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_set_brightness(struct led_base *me, uint8_t brightness);

/* GPIO 子类：base 故意不放第一个位置
 *
 * magic 字段挡在 base 前面，让 offsetof(struct led_gpio, base) != 0。
 * 由于 struct led_base 第一个成员是 const struct led_ops *ops 指针，
 * 32 位机器对齐 4 字节，编译器会在 magic 后面加 2 字节 padding，
 * base 落到偏移 4。运行时 demo 会打印 "offsetof(...,base) = 4" 验证。
 * 详见 ch13 § 13.8.1 编译器算偏移的过程。
 */
struct led_gpio {
	uint16_t        magic;     /* 故意挡在前面，让 base 的偏移不为 0 */
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
