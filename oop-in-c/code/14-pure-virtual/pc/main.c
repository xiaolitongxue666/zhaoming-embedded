/* SPDX-License-Identifier: MIT */
/*
 * main.c - 演示三种 ops 表策略
 *
 * 1. GPIO 灯: 必填 + 选填混合. on / off 子类填了, set_brightness 故意没填,
 *    应用层调 led_set_brightness 走父类默认行为 (安静跳过), 不崩.
 * 2. PWM 灯: 必填 + 选填混合. 三件套全填, 每个调用都走子类实现.
 * 3. temp_sensor: 全必填接口. 三件套全填, sensor_xxx 三个统一接口都能进
 *    子类.
 *
 * 想体验"忘了填 on"的崩溃: 把 led_gpio_init 里的 led_base_init 调用注释
 * 掉再编译, led_on 里的 assert 立刻报错 (因为 me->ops 没装上).
 */

#include "led.h"
#include "led_gpio.h"
#include "led_pwm.h"
#include "sensor.h"
#include "platform.h"
#include <stdio.h>

int main(void)
{
	struct led_gpio    g_led_err;
	struct led_pwm     g_led_stat;
	struct temp_sensor g_temp;
	int32_t            v = 0;
	int                rc;

	printf("=========================================\n");
	printf("  ch14 - pure virtual / virtual / interface\n");
	printf("=========================================\n");

	/* 子类 init 把对应的 const ops 表交给 base, 一次填好 */
	rc = led_gpio_init(&g_led_err, "ERR", PIN_NUM('A', 13), true);
	if (rc != 0) {
		printf("led_gpio_init failed: %d\n", rc);
		return 1;
	}

	rc = led_pwm_init(&g_led_stat, "STAT", 1, 50);
	if (rc != 0) {
		printf("led_pwm_init failed: %d\n", rc);
		return 1;
	}

	rc = temp_sensor_init(&g_temp, "TEMP");
	if (rc != 0) {
		printf("temp_sensor_init failed: %d\n", rc);
		return 1;
	}

	printf("\n--- 1. GPIO LED, no dimming support ---\n");
	led_on(&g_led_err.base);
	led_off(&g_led_err.base);
	/* set_brightness 没填, 父类统一接口走默认行为, 不崩 */
	led_set_brightness(&g_led_err.base, 50);

	printf("\n--- 2. PWM LED, full ops ---\n");
	led_on(&g_led_stat.base);
	led_set_brightness(&g_led_stat.base, 70);
	led_off(&g_led_stat.base);

	printf("\n--- 3. sensor, all required ---\n");
	sensor_self_test(&g_temp.base);
	sensor_calibrate(&g_temp.base);
	sensor_read(&g_temp.base, &v);

	printf("\n=========================================\n");
	printf("  3 strategies: required / optional / contract\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
