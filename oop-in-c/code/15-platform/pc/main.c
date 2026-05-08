/* SPDX-License-Identifier: MIT */
/*
 * main.c - ch15 完整 LED 框架演示 (风格 A)
 *
 * 整本书 ch01 - ch14 一路学的所有 OOP 武器, 在这里组装成一份完整工程:
 *
 *   父类层 (led_base.h, led_base.c)   : led_base + ops 表 + led_base_init
 *                                       + 父类统一接口 led_on / led_off /
 *                                       led_set_brightness
 *   子类层 (led_gpio.{h,c} /          : led_gpio / led_pwm / led_i2c, 每个
 *           led_pwm.{h,c}  /            子类一对独立文件, container_of 反推
 *           led_i2c.{h,c})
 *   平台层 (platform_init.c)          : 把 PC 后端 platform_pwm / platform_i2c
 *                                       的 ops 注册进 dispatcher (所有外设共用)
 *   板级层 (leds.h, led_board_init.c) : LED 模块的硬件参数 + 三种子类混搭 +
 *                                       向上转型挂全局句柄
 *   应用层 (app.h, app.c)             : alarm_blink / status_indicate /
 *                                       power_on_test
 *
 * 启动顺序: platform_init() -> led_board_init() -> 应用代码. 真实工程
 * 一块板上还有 sensor / uart / motor 等等, 每个外设各自一份
 * xxx_board_init.c 在 platform_init() 之后调一次, 都是同款风格.
 *
 * 主线: grep app.c 拿不到任何硬件字样 (led_gpio / led_pwm / led_i2c /
 * gpio_write 全部 0 命中). 应用层只用 led_base * 句柄. 换硬件方案 ->
 * 改 led_board_init.c 三行, app.c 0 改动. 这就是 ch15 章末金句:
 *
 *   好的架构不是让你写更多代码, 是让你改更少代码.
 *
 * 见 ch15 § 15.5 应用层 + § 15.6 换硬件 diff.
 */

#include "app.h"
#include "leds.h"
#include "platform_init.h"
#include <stdio.h>

int main(void)
{
	int rc;

	printf("=========================================\n");
	printf("  ch15 - OOP complete framework demo\n");
	printf("=========================================\n");

	rc = platform_init();
	if (rc != 0) {
		printf("[main] platform_init failed, rc=%d, abort.\n", rc);
		return 1;
	}

	rc = led_board_init();
	if (rc != 0) {
		printf("[main] led_board_init failed, rc=%d, abort.\n", rc);
		return 1;
	}

	power_on_test();
	alarm_blink();
	status_indicate(0);   /* 正常 -> 状态灯 */
	status_indicate(1);   /* 故障 -> 报警灯 */

	printf("\n=========================================\n");
	printf("  app.c never named any hardware type\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
