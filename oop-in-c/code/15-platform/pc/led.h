/* SPDX-License-Identifier: MIT */
/*
 * led.h - 子类 + ops 表 (ch15 完整版, 风格 A)
 *
 * 这一份头跟 ch10 / ch11 一脉相承: 应用层 #include "leds.h" -> "led.h",
 * 看到的是父类指针 struct led_base *, 不直接 #include 子类头. 子类的
 * 结构体定义和 init 声明集中在这一份, 但应用层不会去取 struct led_gpio
 * 类型 -- 它走 g_led_xxx 这种全局 base 句柄.
 *
 * ch15 把 ch07 - ch14 学过的所有 OOP 武器组装在这一份子类层:
 *   - struct led_base + ops 表 (ch11 多态完整图景)
 *   - 子类把 base 嵌进去做"继承" (ch06)
 *   - 父类统一接口走 ops dispatch (ch11)
 *   - 必填 / 选填混用 (ch14: on / off 必填, set_brightness 选填)
 *   - 子类实现里 container_of 反推 (ch13)
 *   - 子类调 platform_gpio_xxx 封装函数, 从来不碰寄存器 (ch01 - ch10)
 *
 * 见 ch15 § 15.3 子类层 + ch11 § 11.3 子类 / § 11.4 父类统一接口.
 */

#ifndef LED_H
#define LED_H

#include "led_base.h"

/*
 * struct led_ops - 操作表 (ch14 的"必填 + 选填"混合形态).
 *
 * on / off 必填: 子类没填, 父类统一接口里的 assert 立刻报错.
 * set_brightness 选填: 子类没填, 父类统一接口走默认行为 (打印一行
 * "no dimming, skip"), 不崩.
 *
 * 这两种语义对应 C++ 的纯虚函数 (virtual void f() = 0;) 和带默认行为
 * 的虚函数 (virtual void f() { 默认实现 }), 见 ch14 § 14.2 / § 14.3.
 */
struct led_ops {
	int (*on)(struct led_base *me);                 /* 必填 */
	int (*off)(struct led_base *me);                /* 必填 */
	int (*set_brightness)(struct led_base *me,      /* 选填 */
	                      uint8_t brightness);
};

/*
 * 父类统一接口 - 写在 led_base.c, 所有子类共用.
 * 应用层只调 led_on / led_off / led_set_brightness, 看不到 ops 字段.
 *
 * 同 ch11 风格: led_xxx 系列声明在 led.h, 应用层 #include "leds.h" 即可
 * 一并见到; 实现落在 led_base.c, 因为它们是父类层职责.
 */
int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_set_brightness(struct led_base *me, uint8_t brightness);

/*
 * ------- 子类一: GPIO LED -------
 *
 * 最简单的 LED: 一个 GPIO 引脚拉高 (或拉低) 点亮.
 * base 必须在第一个字段 (零开销向上转型, 见 ch12 § 12.2).
 *
 * on_level 让同一份 led_gpio 子类支持两种接法: 高电平点亮 (LED 阴极
 * 接 GPIO, 阳极接 VCC) / 低电平点亮 (反过来, LED 共阳极). 不用为这
 * 两种接法各写一个 led_gpio 子类, bool 字段区分就行.
 */
struct led_gpio {
	struct led_base base;       /* 父类, 第 0 字段 */
	uint8_t         pin;
	bool            on_level;   /* 1 = 高电平点亮, 0 = 低电平点亮 */
};

int led_gpio_init(struct led_gpio *me, const char *name,
                  uint8_t pin, bool on_level);

/*
 * ------- 子类二: PWM LED -------
 *
 * 通过 PWM 占空比驱动. 硬件资源: 一路 PWM 通道 + 当前占空比.
 * 三件套全填, 支持调光.
 */
struct led_pwm {
	struct led_base base;       /* 父类, 第 0 字段 */
	uint8_t         channel;
	uint8_t         duty;
};

int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty);

/*
 * ------- 子类三: I2C 扩展芯片 LED -------
 *
 * 走 I2C 总线给某个寄存器写 0/1 控制一颗 LED 亮灭. 硬件资源:
 * 总线编号 + 7-bit 设备地址.
 *
 * 这种结构在工业控制板里很常见 (主控 GPIO 不够用, 挂一颗 PCA9555
 * 之类的 I/O 扩展芯片). 应用层完全不知道, 它只调 led_on(handle).
 */
struct led_i2c {
	struct led_base base;       /* 父类, 第 0 字段 */
	uint8_t         bus;
	uint8_t         addr;
};

int led_i2c_init(struct led_i2c *me, const char *name,
                 uint8_t bus, uint8_t addr);

#endif /* LED_H */
