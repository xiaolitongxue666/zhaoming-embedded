/* SPDX-License-Identifier: MIT */
/*
 * leds.h - 板级层对外暴露的全局 LED 句柄 (ch15 完整版, 风格 A)
 *
 * 三个 struct led_base * 全局句柄 + board_init() 入口. 注意句柄类型是
 * 父类 led_base *, 不是具体子类. 这是 ch12 向上转型在 ch15 工程里的
 * 落地: 应用层拿到 g_led_error 也分不清它底层是 GPIO 还是 PWM 还是
 * I2C.
 *
 * 应用层 #include "leds.h" 一行就够, 不直接 #include "led_gpio.h" /
 * "led_pwm.h" / "led_i2c.h" -- 那些子类头里有 struct led_gpio /
 * led_pwm / led_i2c 的子类定义, 应用层不该去看. 见 ch15 § 15.4 板级
 * 层 + § 15.9.1 应用层 include 纪律.
 *
 * leds.h 只 #include "led_base.h" 拿到 struct led_base 类型, 应用层
 * 看到的就是父类指针; 板级文件 board_init.c 例外, 它要看到子类完整
 * 类型才能 sizeof 占栈, 在那里另行 #include 子类头.
 */

#ifndef LEDS_H
#define LEDS_H

#include "led_base.h"

extern struct led_base *g_led_error;
extern struct led_base *g_led_status;
extern struct led_base *g_led_network;

/*
 * board_init - 板级初始化入口.
 *
 * 返回 0 表示三盏灯全部初始化成功 + 三个全局句柄已绑定; < 0 表示
 * 某盏灯 init 失败 (参数非法或硬件资源不对), main 收到立刻退出.
 */
int board_init(void);

#endif /* LEDS_H */
