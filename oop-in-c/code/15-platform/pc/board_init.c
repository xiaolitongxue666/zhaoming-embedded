/* SPDX-License-Identifier: MIT */
/*
 * board_init.c - 板级硬件配置
 *
 * 全工程唯一一份"认识具体硬件"的文件——pin 编号、PWM 通道、I2C 地址
 * 这些常量都集中在这里。三种子类混搭：ERR 是 GPIO LED、STAT 是 PWM
 * 灯（支持调光）、NET 是 I2C 灯。
 *
 * 应用层只看到 g_led_error / g_led_status / g_led_network 这三个
 * struct led_base * 句柄，看不到 GPIO/PWM/I2C 的差别——这是 ch12 的
 * 向上转型在 ch15 完整工程里的应用。换一块板（pin 换、通道换、I2C
 * 地址换），只改这一个文件，应用层 0 改动。
 *
 * 见 ch15 § 15.4 板级层 + ch12 § 12.x 向上转型。
 */

#include "leds.h"

static struct led_gpio s_led_err;
static struct led_pwm  s_led_status;
static struct led_i2c  s_led_net;

struct led_base *g_led_error;
struct led_base *g_led_status;
struct led_base *g_led_network;

void board_init(void)
{
	led_gpio_init(&s_led_err,    "ERR",  10, true);
	led_pwm_init (&s_led_status, "STAT",  1, 50);
	led_i2c_init (&s_led_net,    "NET",   0, 0x20);

	g_led_error   = &s_led_err.base;
	g_led_status  = &s_led_status.base;
	g_led_network = &s_led_net.base;
}
