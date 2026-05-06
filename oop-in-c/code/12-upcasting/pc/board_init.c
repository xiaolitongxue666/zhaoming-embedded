/* SPDX-License-Identifier: MIT */
/**
 * @file  board_init.c
 * @brief 整个工程里唯一认识硬件的文件 - 板级初始化 (BSP 核心)
 *
 * @details
 * 见 ch12 § 12.4. 嵌入式工程里这种文件叫板级初始化 (Board Support
 * Package, BSP). 每块板子一份 board_init.c, 配置不同板子上 LED
 * 接的是 GPIO 还是 PWM 还是 I2C 扩展芯片.
 *
 * 应用层看到三个 struct led_base * 句柄 (在 leds.h extern 声明),
 * 硬件配置全部锁在本文件:
 *   - 子类对象 s_led_err / s_led_status / s_led_net 文件作用域 static
 *     (外部不可见, 防止其他模块绕过基类指针直接访问硬件字段)
 *   - 全局句柄 g_led_error / g_led_status / g_led_network 是
 *     struct led_base * 类型, 在 board_init() 里被赋值 &xxx.base
 *
 * 换硬件方案 (例如报警灯 GPIO -> PWM) 只需改本文件三行 (子类类型、
 * init 调用、句柄绑定), main.c 一字不动. 见 ch12 § 12.6.
 *
 * board_init() 在 main 里开机调一次. 真实工程会有更长的初始化
 * 链 (时钟、中断向量、RTOS, 等等).
 */

#include "leds.h"

/*
 * 子类对象 - 文件作用域 + static, 外部不可见.
 *
 * 这一层 static 是关键: 其他模块即使想 #include "led.h" 也拿不到
 * s_led_err 的地址, 只能通过 g_led_error 这个 base 句柄间接访问.
 * 应用层永远不会"碰巧"把 struct led_gpio * 漏出来.
 *
 * 三个对象类型完全不同 (GPIO / PWM / I2C), 但因为它们各自把
 * struct led_base 嵌在第 0 字段, &xxx.base 拿到的地址就能当
 * 通用 struct led_base * 句柄用. 这就是向上转型.
 */
static struct led_gpio s_led_err;
static struct led_pwm  s_led_status;
static struct led_i2c  s_led_net;

/*
 * 全局句柄 - 真正的"定义". 绑定动作在 board_init 里完成,
 * 这三行此刻只是占位 (BSS 段默认初始化为 NULL).
 *
 * 类型是 struct led_base *, 应用层 #include "leds.h" 之后看到的
 * extern 声明就是这一组. 应用层永远不知道背后挂的是哪种子类.
 */
struct led_base *g_led_error;
struct led_base *g_led_status;
struct led_base *g_led_network;

void board_init(void)
{
	/*
	 * 各自走自己的子类构造函数, 把硬件资源传进去.
	 * 子类 init 内部把对应的 ops 表挂到 base 字段.
	 *
	 * 三种 LED 三种硬件:
	 *   GPIO 灯 (ERR)   -> pin = 10, on_level = high
	 *   PWM  灯 (STAT)  -> channel = 1, duty = 50%
	 *   I2C  灯 (NET)   -> bus = 0, addr = 0x20
	 */
	led_gpio_init(&s_led_err,    "ERR",  10, true);
	led_pwm_init (&s_led_status, "STAT",  1, 50);
	led_i2c_init (&s_led_net,    "NET",   0, 0x20);

	/*
	 * 关键三行: 把子类对象的 base 地址, 赋给父类指针句柄.
	 * 这就是向上转型 (upcasting), 子类对象当父类来用.
	 *
	 * 为什么合法 (见 ch12 § 12.2 / § 12.8.1):
	 *   C99 § 6.7.2.1 第 13 段保证"结构体第一个成员的地址等于
	 *   结构体本身的地址". 因为 base 是子类的第一个字段,
	 *       &s_led_err == &s_led_err.base    (作为指针值)
	 *   两个指针类型不同, 但指向同一个字节. 编译器替我们算偏移
	 *   (本例偏移是 0, 一条加法指令都不会生成, 零开销).
	 *
	 * 为什么写 &s_led_err.base 而不是 (struct led_base *)&s_led_err
	 * (见 ch12 § 12.8.1):
	 *   .base 字段访问让编译器自己算偏移, base 不管在第几个位置都对.
	 *   强转写法假定"base 永远在第一个", 哪天某个倒霉的同事把 base
	 *   挪到第二个字段, 强转代码立刻崩, 字段访问代码自动适应.
	 */
	g_led_error   = &s_led_err.base;
	g_led_status  = &s_led_status.base;
	g_led_network = &s_led_net.base;
}
