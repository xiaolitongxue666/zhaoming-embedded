/* SPDX-License-Identifier: MIT */
/*
 * board_init.c - 板级硬件配置 (ch15 完整版, 风格 A)
 *
 * 全工程唯一一份"认识具体硬件"的文件 (BSP 核心). pin 编号、PWM 通道、
 * I2C bus + client 地址这些常量都集中在这里. 三种子类混搭: ERR 是 GPIO
 * 灯、STAT 是 PWM 灯 (支持调光)、NET 是 I2C 灯.
 *
 * 应用层只看到 g_led_error / g_led_status / g_led_network 这三个
 * struct led_base * 句柄, 看不到 GPIO / PWM / I2C 的差别. 这是 ch12
 * 向上转型在 ch15 完整工程里的应用. 换一种硬件方案 (例如报警灯
 * GPIO -> PWM), 只需改本文件三行 (子类类型、init 调用、句柄绑定),
 * app.c 0 改动.
 *
 * I2C 这一路 ch15 升级到了 bus + client 二层 (见 § 15.17.2):
 *   1) 启动期先调 platform_pc_i2c_init 把 PC bus 注册进 platform_i2c
 *      dispatcher, 拿到 bus 句柄.
 *   2) 把 bus 句柄喂给 led_i2c_init, 子类内嵌的 struct platform_i2c_client
 *      字段 (bus + client_addr) 一次填好.
 *   3) 之后 led_on / led_off 拼 msg 走 platform_i2c_transfer, 经 bus 控
 *      制器层 dispatch 到 PC 后端 printf 出 [I2C] addr=0x3C W len=2 ...
 *      这一行就是二层兑现的可视化输出.
 *
 * 见 ch15 § 15.4 板级层 + § 15.6 换硬件 diff + § 15.17.2 / § 15.17.3.
 */

#include "leds.h"
#include "led_gpio.h"
#include "led_pwm.h"
#include "led_i2c.h"
#include "platform/platform_i2c.h"
#include <stdio.h>

/* PC 端 platform_pwm / platform_i2c 注册函数, 同款 ops 表注册风格,
 * 实现分别在 platform_pwm_pc.c / platform_i2c_pc.c. */
extern void platform_pc_pwm_init(void);
extern void platform_pc_i2c_init(void);

/*
 * 子类对象 - 文件作用域 + static, 外部不可见.
 *
 * 这一层 static 是关键: 其他模块即使想 #include "led_gpio.h" 也拿不到
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

int board_init(void)
{
	struct platform_i2c_bus_device *i2c_bus;
	int rc;

	/*
	 * 第一步: 把 PC 后端的 platform_pwm / platform_i2c ops 注册进
	 * dispatcher. STM32 端这一步在 platform/arch/stm32/pin_board.c
	 * 的 platform_hw_pin_init 里做, NXP 端在 arch/nxp/pin_board.c 里做.
	 * PC 端拆成 platform_pwm_pc.c + platform_i2c_pc.c 两份小文件.
	 * 三处都是同一个 platform_xxx_register, 注册的 ops 表不同而已.
	 *
	 * platform_gpio (common/platform_pc.c) 是 ch01-ch14 一路用下来的
	 * 简版 (直接定义 platform_gpio_init / write 4 个函数, 不走 ops 表
	 * + register), 教学渐进的产物. ch15 章节正文 § 15.7 / § 15.11 说明
	 * 真实工业项目这一层也升 ops 表 + register, 见 platform_pin.h.
	 */
	platform_pc_pwm_init();
	platform_pc_i2c_init();

	i2c_bus = platform_i2c_bus_get();
	if (!i2c_bus) {
		printf("[board] platform_i2c_bus_get returned NULL\n");
		return -1;
	}

	/*
	 * 三种 LED 三种硬件参数, 全集中在这一个 board_init 里写死:
	 *   GPIO 灯 (ERR)   -> pin 10, on_level = high
	 *   PWM  灯 (STAT)  -> channel 1, duty 50%
	 *   I2C  灯 (NET)   -> bus = pc_i2c_bus, client_addr 0x3C, reg 0x00
	 *
	 * 子类 init 返回 int: 0 = 成功, < 0 = 参数非法或硬件资源不对.
	 * 任何一盏灯 init 失败立刻返回, 别静默吞错 -- 真实板子上漏了一盏
	 * 灯的初始化, 应用跑起来时才发现 led_on 走 NULL ops 直接 abort,
	 * 不如开机时就拒绝起来.
	 */
	rc = led_gpio_init(&s_led_err, "ERR", 10, true);
	if (rc != 0) {
		printf("[board] led_gpio_init(ERR) failed, rc=%d\n", rc);
		return rc;
	}

	rc = led_pwm_init(&s_led_status, "STAT", 1, 50);
	if (rc != 0) {
		printf("[board] led_pwm_init(STAT) failed, rc=%d\n", rc);
		return rc;
	}

	rc = led_i2c_init(&s_led_net, "NET", i2c_bus, 0x3C, 0x00);
	if (rc != 0) {
		printf("[board] led_i2c_init(NET) failed, rc=%d\n", rc);
		return rc;
	}

	/*
	 * 关键三行: 把子类对象的 base 地址赋给父类指针句柄 (向上转型).
	 * C99 § 6.7.2.1: 结构体第一个成员的地址等于结构体本身的地址,
	 * 所以 &s_led_err.base 和 &s_led_err 指向同一个字节, 类型不同
	 * 而已. 见 ch12 § 12.2 / § 12.8.1.
	 *
	 * 写 &s_led_err.base 而不是 (struct led_base *)&s_led_err 的好处:
	 * 字段访问让编译器自己算偏移, base 不管在第几个位置都对; 强转
	 * 写法假定 base 永远在第一个, 哪天某个倒霉的同事把 base 挪到
	 * 第二个字段, 强转代码立刻崩, 字段访问代码自动适应.
	 */
	g_led_error   = &s_led_err.base;
	g_led_status  = &s_led_status.base;
	g_led_network = &s_led_net.base;
	return 0;
}
