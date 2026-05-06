/* SPDX-License-Identifier: MIT */
/*
 * led.h - 三种 ops 表策略：必填 / 选填 / 全必填
 *
 * 这一章解决一颗雷：ch11 / ch13 的 ops 表里，如果子类某个函数指针忘填，
 * C 标准说静态存储未显式初始化的字段会被零初始化，也就是 NULL。
 * 应用层调 led_on(handle)，进到 me->ops->on(me) 把 NULL 当函数地址跳，
 * STM32 上一般 HardFault 死循环、Linux 用户态 SIGSEGV 进程当场死。
 * 编译器一句话不说就让你过了，它觉得你是故意填 NULL 的。
 *
 * 三种处理策略对应 C++ 三种语义：
 *   必填（纯虚函数）：on / off。子类不填，led_on 内部 assert 失败。
 *                    对应 C++ 的 virtual void f() = 0; 抽象类不能实例化。
 *   选填（带默认行为的虚函数）：set_brightness。子类不填，统一接口默认跳过。
 *                    对应 C++ 的 virtual void f() { 默认实现 } 子类可覆写。
 *   全必填（接口）：struct sensor_ops 三件套全 assert。这是 C 模拟"接口"
 *                    的形态，对应 Java 的 interface / C++ 全纯虚 class。
 *
 * 见 ch14 § 14.2 / 14.3 / 14.4。
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

/* GPIO 子类：只填 on / off，set_brightness 留空（不支持调光） */
struct led_gpio {
	struct led_base base;
	uint8_t         pin;
	bool            on_level;
};

void led_gpio_init(struct led_gpio *me, const char *name,
		   uint8_t pin, bool on_level);

/* PWM 子类：三件套全填 */
struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};

void led_pwm_init(struct led_pwm *me, const char *name,
		  uint8_t channel, uint8_t duty);

/* ============== 接口（全必填）示例 ============== */

struct sensor;

struct sensor_ops {
	int (*read)(struct sensor *me, int32_t *out);   /* 全必填 */
	int (*calibrate)(struct sensor *me);            /* 全必填 */
	int (*self_test)(struct sensor *me);            /* 全必填 */
};

struct sensor {
	const struct sensor_ops *ops;
	const char              *name;
};

int sensor_read(struct sensor *me, int32_t *out);
int sensor_calibrate(struct sensor *me);
int sensor_self_test(struct sensor *me);

#endif /* LED_H */
