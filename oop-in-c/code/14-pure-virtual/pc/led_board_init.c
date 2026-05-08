/* SPDX-License-Identifier: MIT */
/**
 * @file  led_board_init.c
 * @brief LED 模块板级配置 - 全工程唯一认识 LED 硬件的文件
 *
 * @details
 * ch14 在 ch12 / ch13 封装的基础上一字不退: 应用层 main.c 只 include
 * leds.h, 看到的全是 struct led_base * 句柄.
 *
 * 本章主题三种 ops 策略是子类 + 父类统一接口的事, 跟封装层无关:
 *   - GPIO 子类: on/off 必填, set_brightness 选填(不填), 父类统一接口
 *     在 NULL 的 set_brightness 上走默认行为(安静跳过, 不崩)
 *   - PWM  子类: 三件套全填, 调用走子类实现
 *
 * 命名带模块前缀 led_board_init 而不是一个全局 board_init, 是因为真实
 * 工程一块板上还有 sensor / uart / motor 等等, 每个外设各自一份
 * xxx_board_init.c. 本章 sensor 也走自己的 sensor_board_init.c.
 */

#include "leds.h"
#include "led_gpio.h"
#include "led_pwm.h"
#include "platform.h"
#include <stdio.h>

/* 子类对象 - 文件作用域 + static, 外部不可见. */
static struct led_gpio s_led_err;
static struct led_pwm  s_led_status;

/* 全局句柄 - struct led_base * 类型, 应用层 #include "leds.h" 看到的就是
 * 这一组 extern 声明. */
struct led_base *g_led_error;
struct led_base *g_led_status;

int led_board_init(void)
{
	int rc;

	/*
	 * GPIO 灯 (ERR)   -> PA.13, on_level = high  (没填 set_brightness)
	 * PWM  灯 (STAT)  -> channel = 1, duty = 50% (三件套全填)
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

	g_led_error  = &s_led_err.base;
	g_led_status = &s_led_status.base;
	return 0;
}
