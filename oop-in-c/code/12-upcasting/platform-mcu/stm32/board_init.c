/* SPDX-License-Identifier: MIT */
/*
 * board_init.c - STM32 板级初始化片段 (BSP 核心)
 *
 * 这是片段, 不是完整工程. 完整 STM32 工程见附录 B.
 *
 * 跟 pc/board_init.c 99% 一致, 唯一差别: GPIO 子类的 pin 参数从 PC 版的
 * 占位 10 换成真实板子上的 PIN_NUM('A', 13). 子类 init / 句柄绑定 /
 * 全局句柄声明全部一字不改.
 *
 * 换板子 (LED 接到 PD.12 而不是 PA.13) 只需改这一行 pin 参数, main.c /
 * led.c / led_*.c 一字不动. 这就是 board_init.c 的工程意义.
 */

#include "leds.h"
#include "led_gpio.h"
#include "led_pwm.h"
#include "led_i2c.h"
#include "platform.h"

/* 子类对象 - 文件作用域 + static, 外部不可见 (见 pc/board_init.c 注释). */
static struct led_gpio s_led_err;
static struct led_pwm  s_led_status;
static struct led_i2c  s_led_net;

/* 全局句柄 - struct led_base * 类型, 应用层 #include "leds.h" 看到的就是
 * 这一组 extern 声明. */
struct led_base *g_led_error;
struct led_base *g_led_status;
struct led_base *g_led_network;

int board_init(void)
{
	int rc;

	/*
	 * 三种 LED 三种硬件:
	 *   GPIO 灯 (ERR)   -> PA.13, on_level = high
	 *   PWM  灯 (STAT)  -> TIM3 channel 1, duty = 50%
	 *   I2C  灯 (NET)   -> bus = 0, addr = 0x20
	 */
	rc = led_gpio_init(&s_led_err, "ERR", PIN_NUM('A', 13), true);
	if (rc != 0)
		return rc;

	rc = led_pwm_init(&s_led_status, "STAT", 1, 50);
	if (rc != 0)
		return rc;

	rc = led_i2c_init(&s_led_net, "NET", 0, 0x20);
	if (rc != 0)
		return rc;

	/* 关键三行: 把子类对象的 base 地址赋给父类指针句柄 (向上转型). */
	g_led_error   = &s_led_err.base;
	g_led_status  = &s_led_status.base;
	g_led_network = &s_led_net.base;

	return 0;
}
