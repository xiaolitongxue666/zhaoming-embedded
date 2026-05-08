/* SPDX-License-Identifier: MIT */
/*
 * main.c - 应用层
 *
 * ch14 主题三种 ops 策略 (必填 / 选填 / 接口) 是子类内部 + 父类统一接口
 * 的事, 跟应用层封装层无关. 本章和 ch12 / ch13 保持同一种封装: 应用层
 * 只 include leds.h + sensors.h, 看到的全是 struct led_base * /
 * struct sensor_base * 句柄, 子类一字看不到.
 *
 * 三段演示走父类统一接口:
 *   1. led_on/off + led_set_brightness  on g_led_error  (GPIO 没填 set_br)
 *   2. led_on/off + led_set_brightness  on g_led_status (PWM 三件套全填)
 *   3. sensor_self_test/calibrate/read  on g_temp_sensor (sensor 全必填)
 *
 * 想体验"忘了填 on"的崩溃: 把 led_gpio_init 里的 led_base_init 调用注释
 * 掉再编译, led_on 里的 assert 立刻报错 (因为 me->ops 没装上).
 */

#include "leds.h"
#include "sensors.h"
#include <stdio.h>

int main(void)
{
	int     rc;
	int32_t v = 0;

	printf("=========================================\n");
	printf("  ch14 - pure virtual / virtual / interface\n");
	printf("=========================================\n");

	rc = led_board_init();
	if (rc != 0)
		return rc;

	rc = sensor_board_init();
	if (rc != 0)
		return rc;

	printf("\n--- 1. GPIO LED, no dimming support ---\n");
	led_on(g_led_error);
	led_off(g_led_error);
	/* set_brightness 没填, 父类统一接口走默认行为, 不崩 */
	led_set_brightness(g_led_error, 50);

	printf("\n--- 2. PWM LED, full ops ---\n");
	led_on(g_led_status);
	led_set_brightness(g_led_status, 70);
	led_off(g_led_status);

	printf("\n--- 3. sensor, all required ---\n");
	sensor_self_test(g_temp_sensor);
	sensor_calibrate(g_temp_sensor);
	sensor_read(g_temp_sensor, &v);

	printf("\n=========================================\n");
	printf("  3 strategies: required / optional / contract\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
