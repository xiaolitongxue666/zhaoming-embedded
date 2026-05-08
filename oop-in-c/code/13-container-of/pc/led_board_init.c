/* SPDX-License-Identifier: MIT */
/**
 * @file  led_board_init.c
 * @brief LED 模块板级配置 - 全工程唯一认识 LED 硬件的文件
 *
 * @details
 * ch13 在 ch12 封装的基础上一字不退: 应用层 main.c 只 include leds.h,
 * 看到的全是 struct led_base * 句柄. ch13 主题 container_of 是子类内部
 * 的事 (子类 .c 用 container_of 把 base * 反推回子类指针), 跟应用层无关.
 *
 * 本文件做三件事:
 *   1. 实例化三个子类对象 (file-scope static, 应用层不可见)
 *   2. 启动期打印三种子类的 offsetof(..., base), 让 GPIO 那个故意非 0
 *      的偏移肉眼可见 -- 这是 ch13 § 13.8.2 位置无关 demo 的关键观察点
 *   3. 调三个子类 init + 把 &xxx.base 赋给三个父类指针句柄 (向上转型)
 *
 * 命名带模块前缀 led_board_init 而不是一个全局 board_init, 是因为真实
 * 工程一块板上还有 sensor / uart / motor 等等, 每个外设各自一份
 * xxx_board_init.c, 谁的硬件参数谁负责.
 */

#include "leds.h"
#include "led_gpio.h"
#include "led_pwm.h"
#include "led_i2c.h"
#include "platform.h"
#include <stddef.h>
#include <stdio.h>

/*
 * 子类对象 - 文件作用域 + static, 外部不可见.
 *
 * GPIO 故意把 base 挪到第二个字段, 让 offsetof(struct led_gpio, base) != 0.
 * 子类内部用 container_of 反推自己, 不依赖 base 在 0 偏移这个假设.
 * 见 ch13 § 13.8.2.
 */
static struct led_gpio s_led_err;
static struct led_pwm  s_led_status;
static struct led_i2c  s_led_net;

/*
 * 全局句柄 - 真正的"定义". 类型是 struct led_base *, 应用层
 * #include "leds.h" 看到的就是这一组 extern 声明.
 */
struct led_base *g_led_error;
struct led_base *g_led_status;
struct led_base *g_led_network;

int led_board_init(void)
{
	int rc;

	/*
	 * 启动期打印一次三种子类的 base 偏移. GPIO 偏移 != 0 (32-bit 平台 4,
	 * 64-bit 平台 8), PWM / I2C 偏移 = 0. 这是 ch13 主题 container_of 与
	 * 位置无关性的肉眼证据.
	 */
	printf("[led_board] offsetof(struct led_gpio, base) = %u\n",
	       (unsigned)offsetof(struct led_gpio, base));
	printf("[led_board] offsetof(struct led_pwm,  base) = %u\n",
	       (unsigned)offsetof(struct led_pwm, base));
	printf("[led_board] offsetof(struct led_i2c,  base) = %u\n",
	       (unsigned)offsetof(struct led_i2c, base));

	/*
	 * 三种 LED 三种硬件 (PC 版用占位参数):
	 *   GPIO 灯 (ERR)   -> PA.13, on_level = high
	 *   PWM  灯 (STAT)  -> channel = 1, duty = 50%
	 *   I2C  灯 (NET)   -> bus = 0, addr = 0x20
	 */
	rc = led_gpio_init(&s_led_err, "ERR", PIN_NUM('A', 13), true);
	if (rc != 0) {
		printf("[led_board] led_gpio_init(ERR) failed, rc=%d\n", rc);
		return rc;
	}

	rc = led_pwm_init(&s_led_status, "STAT", 1, 50);
	if (rc != 0) {
		printf("[led_board] led_pwm_init(STAT) failed, rc=%d\n", rc);
		return rc;
	}

	rc = led_i2c_init(&s_led_net, "NET", 0, 0x20);
	if (rc != 0) {
		printf("[led_board] led_i2c_init(NET) failed, rc=%d\n", rc);
		return rc;
	}

	/*
	 * 关键三行: 把子类对象的 base 地址赋给父类指针句柄 (向上转型).
	 * 子类内部 on/off/set_brightness 拿到 me 之后, 用 container_of 反推
	 * 回子类指针. GPIO 那一路偏移非 0 也对, 因为 container_of 是编译器
	 * 算偏移, 不是假设 base 在第一个.
	 */
	g_led_error   = &s_led_err.base;
	g_led_status  = &s_led_status.base;
	g_led_network = &s_led_net.base;
	return 0;
}
