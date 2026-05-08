/* SPDX-License-Identifier: MIT */
/*
 * led_board_init.c - LED 模块板级配置 (ch15 完整版, 风格 A)
 *
 * 全工程唯一一份"认识 LED 硬件"的文件 (BSP 核心的 LED 那一份). pin
 * 编号、PWM 通道、I2C bus + client 地址这些 LED 硬件常量集中在这里.
 * 三种子类混搭: ERR 是 GPIO 灯、STAT 是 PWM 灯 (支持调光)、NET 是 I2C 灯.
 *
 * 命名带模块前缀 led_board_init 而不是一个全局 board_init, 是因为真实
 * 工程一块板上还有 sensor / uart / motor 等等, 每个外设各自一份
 * xxx_board_init.c, 谁的硬件参数谁负责. ch12 / ch13 / ch14 起就是
 * 这个工程纪律, 这里继续. platform 层 ops 表的注册 (platform_pc_pwm_init
 * / platform_pc_i2c_init) 拆出去到 platform_init.c, 那一层给所有外设
 * 共用, 不归 LED 单独管.
 *
 * 应用层只看到 g_led_error / g_led_status / g_led_network 这三个
 * struct led_base * 句柄, 看不到 GPIO / PWM / I2C 的差别. 这是 ch12
 * 向上转型在 ch15 完整工程里的应用. 换一种硬件方案 (例如报警灯
 * GPIO -> PWM), 只需改本文件三行 (子类类型、init 调用、句柄绑定),
 * app.c 0 改动.
 *
 * I2C 这一路 ch15 升级到了 bus + client 二层 (见 § 15.17.2):
 *   1) platform_init() 里把 PC bus 注册进 platform_i2c dispatcher
 *   2) led_board_init 调 platform_i2c_bus_get 拿 bus 句柄
 *   3) 把 bus 句柄喂给 led_i2c_init, 子类内嵌的 struct platform_i2c_client
 *      字段 (bus + client_addr) 一次填好.
 *   4) 之后 led_on / led_off 拼 msg 走 platform_i2c_transfer, 经 bus 控
 *      制器层 dispatch 到 PC 后端 printf 出 [I2C] addr=0x3C W len=2 ...
 *
 * 见 ch15 § 15.4 板级层 + § 15.6 换硬件 diff + § 15.17.2 / § 15.17.3.
 */

#include "leds.h"
#include "led_gpio.h"
#include "led_pwm.h"
#include "led_i2c.h"
#include "platform/platform_i2c.h"
#include <stdio.h>

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
 * 全局句柄 - 真正的"定义". 绑定动作在 led_board_init 里完成, 这三行
 * 此刻只是占位 (BSS 段默认初始化为 NULL). 类型是 struct led_base *,
 * 应用层 #include "leds.h" 之后看到的 extern 声明就是这一组.
 */
struct led_base *g_led_error;
struct led_base *g_led_status;
struct led_base *g_led_network;

int led_board_init(void)
{
	struct platform_i2c_bus_device *i2c_bus;
	int rc;

	/*
	 * 调 led_board_init 之前 main 已经调过 platform_init(), platform_pwm /
	 * platform_i2c 的 ops 注册已经做完, 这里只管拿 bus 句柄.
	 */
	i2c_bus = platform_i2c_bus_get();
	if (!i2c_bus) {
		printf("[led_board] platform_i2c_bus_get returned NULL\n");
		return -1;
	}

	/*
	 * 三种 LED 三种硬件参数, 全集中在这一个 led_board_init 里写死:
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
		printf("[led_board] led_gpio_init(ERR) failed, rc=%d\n", rc);
		return rc;
	}

	rc = led_pwm_init(&s_led_status, "STAT", 1, 50);
	if (rc != 0) {
		printf("[led_board] led_pwm_init(STAT) failed, rc=%d\n", rc);
		return rc;
	}

	rc = led_i2c_init(&s_led_net, "NET", i2c_bus, 0x3C, 0x00);
	if (rc != 0) {
		printf("[led_board] led_i2c_init(NET) failed, rc=%d\n", rc);
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
