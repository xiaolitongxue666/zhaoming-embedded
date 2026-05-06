/* SPDX-License-Identifier: MIT */
/*
 * board_init.c - 板级硬件配置
 *
 * 全工程唯一一份"认识具体硬件"的文件. pin 编号、PWM 通道、I2C 地址
 * 这些常量都集中在这里. 三种子类混搭: ERR 是 GPIO 灯、STAT 是 PWM
 * 灯 (支持调光)、NET 是 I2C 灯.
 *
 * 应用层只看到 g_led_error / g_led_status / g_led_network 这三个
 * struct led_base * 句柄, 看不到 GPIO / PWM / I2C 的差别. 这是 ch12
 * 的向上转型在 ch15 完整工程里的应用. 换一种硬件方案 (例如报警灯
 * GPIO -> PWM), 只需改本文件三行 (子类类型、init 调用、句柄绑定),
 * app.c 0 改动.
 *
 * 见 ch15 § 15.4 板级层 + § 15.6 换硬件 diff + ch12 向上转型.
 */

#include "leds.h"

/*
 * 子类对象 - 文件作用域 + static, 外部不可见.
 *
 * 这一层 static 是关键: 其他模块即使想 #include "led.h" 也拿不到
 * s_led_err 的地址, 只能通过 g_led_error 这个 base 句柄间接访问.
 * 应用层永远不会"碰巧"把 struct led_gpio * 漏出来.
 */
static struct led_gpio s_led_err;
static struct led_pwm  s_led_status;
static struct led_i2c  s_led_net;

/*
 * 全局句柄 - 真正的"定义". 绑定动作在 board_init 里完成, 这三行
 * 此刻只是占位 (BSS 段默认初始化为 NULL). 类型是 struct led_base *,
 * 应用层 #include "leds.h" 之后看到的 extern 声明就是这一组.
 */
struct led_base *g_led_error;
struct led_base *g_led_status;
struct led_base *g_led_network;

void board_init(void)
{
	/*
	 * 三种 LED 三种硬件参数, 全集中在这一个 board_init 里写死:
	 *   GPIO 灯 (ERR)  -> pin 10, on_level = high
	 *   PWM  灯 (STAT) -> channel 1, duty 50%
	 *   I2C  灯 (NET)  -> bus 0, addr 0x20
	 */
	led_gpio_init(&s_led_err,    "ERR",  10, true);
	led_pwm_init (&s_led_status, "STAT",  1, 50);
	led_i2c_init (&s_led_net,    "NET",   0, 0x20);

	/*
	 * 关键三行: 把子类对象的 base 地址赋给父类指针句柄 (向上转型).
	 * C99 § 6.7.2.1: 结构体第一个成员的地址等于结构体本身的地址,
	 * 所以 &s_led_err.base 和 &s_led_err 指向同一个字节, 类型不同
	 * 而已. 见 ch12 § 12.2 / § 12.8.1.
	 */
	g_led_error   = &s_led_err.base;
	g_led_status  = &s_led_status.base;
	g_led_network = &s_led_net.base;
}
