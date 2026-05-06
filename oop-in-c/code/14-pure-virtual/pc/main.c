/* SPDX-License-Identifier: MIT */
/*
 * main.c - 演示三种 ops 表策略
 *
 * 1. GPIO 灯不支持调光：set_brightness 选填，调到走默认行为，不崩
 * 2. PWM 灯三件套全填：每个调用都走子类实现
 * 3. sensor 三件套全必填：演示 sensor_read 调到一个完整子类的情形
 *
 * 故意演示一段"忘了填 on" 的代码（注释保留），打开 NDEBUG 没开的话会触发 assert。
 */

#include "led.h"
#include "container_of.h"
#include <stdio.h>

/* 一个完整的 sensor 子类：温度传感器 */
struct temp_sensor {
	struct sensor base;
	int32_t       last_value;
};

static int temp_read(struct sensor *me, int32_t *out)
{
	struct temp_sensor *self = container_of(me, struct temp_sensor, base);
	self->last_value = 25;
	*out = self->last_value;
	printf("  [%s] read = %d C\n", me->name, *out);
	return 0;
}

static int temp_calibrate(struct sensor *me)
{
	printf("  [%s] calibrate\n", me->name);
	return 0;
}

static int temp_self_test(struct sensor *me)
{
	printf("  [%s] self_test\n", me->name);
	return 0;
}

static const struct sensor_ops temp_ops = {
	.read      = temp_read,
	.calibrate = temp_calibrate,
	.self_test = temp_self_test,
};

int main(void)
{
	struct led_gpio g_led_err;
	struct led_pwm  g_led_stat;
	struct temp_sensor g_temp;

	led_gpio_init(&g_led_err,  "ERR",  10, true);
	led_pwm_init (&g_led_stat, "STAT",  1, 50);

	g_temp.base.ops  = &temp_ops;
	g_temp.base.name = "TEMP";
	g_temp.last_value = 0;

	printf("\n=========================================\n");
	printf("  ch14 - pure virtual / virtual / interface\n");
	printf("=========================================\n");

	printf("\n--- 1. GPIO LED, no dimming support ---\n");
	led_on(&g_led_err.base);
	led_off(&g_led_err.base);
	/* set_brightness 没填，统一接口走默认行为，不崩 */
	led_set_brightness(&g_led_err.base, 50);

	printf("\n--- 2. PWM LED, full ops ---\n");
	led_on(&g_led_stat.base);
	led_set_brightness(&g_led_stat.base, 70);
	led_off(&g_led_stat.base);

	printf("\n--- 3. sensor, all required ---\n");
	int32_t v = 0;
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
