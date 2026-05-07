/* SPDX-License-Identifier: MIT */
/*
 * main.c - container_of 演示 (ch13)
 *
 * GPIO 子类故意把 base 放到第二个位置, 证明 container_of 与位置无关.
 * ch12 那招 (struct led_gpio *)me 强转在这种布局下会算错地址 4 字节,
 * container_of 一直对.
 *
 * 子类 init 第一行调 led_base_init, ops 表通过共有 init 一次填好,
 * 跟 ch10/ch11 一样. 这里 main 把每个 init 的返回值都接住, 任何一个
 * 子类没初始化好就直接退出, 不让后面的 led_on 拿到野指针.
 */

#include "led.h"
#include "led_gpio.h"
#include "led_pwm.h"
#include "led_i2c.h"
#include "container_of.h"
#include "platform.h"
#include <stddef.h>
#include <stdio.h>

static struct led_gpio g_err   = {0};
static struct led_pwm  g_stat  = {0};
static struct led_i2c  g_net   = {0};

int main(void)
{
	int rc;

	printf("=========================================\n");
	printf("  ch13 - container_of\n");
	printf("=========================================\n");

	/* 让大家看一眼偏移量. GPIO 的 base 故意不在 0 位 */
	printf("offsetof(struct led_gpio, base) = %u\n",
	       (unsigned)offsetof(struct led_gpio, base));
	printf("offsetof(struct led_pwm,  base) = %u\n",
	       (unsigned)offsetof(struct led_pwm, base));
	printf("offsetof(struct led_i2c,  base) = %u\n",
	       (unsigned)offsetof(struct led_i2c, base));
	printf("\n");

	rc = led_gpio_init(&g_err, "ERR", PIN_NUM('A', 13), true);
	if (rc != 0) {
		fprintf(stderr, "led_gpio_init failed: %d\n", rc);
		return 1;
	}
	rc = led_pwm_init(&g_stat, "STAT", 1, 50);
	if (rc != 0) {
		fprintf(stderr, "led_pwm_init failed: %d\n", rc);
		return 1;
	}
	rc = led_i2c_init(&g_net, "NET", 0, 0x20);
	if (rc != 0) {
		fprintf(stderr, "led_i2c_init failed: %d\n", rc);
		return 1;
	}

	struct led_base *handles[] = {
		&g_err.base,
		&g_stat.base,
		&g_net.base,
	};

	for (int i = 0; i < 3; i++) {
		printf("\n--- toggle %s ---\n", handles[i]->name);
		led_on(handles[i]);
		led_off(handles[i]);
	}

	printf("\n--- breath stat ---\n");
	led_set_brightness(handles[1], 60);
	led_set_brightness(handles[1], 0);

	printf("\n=========================================\n");
	printf("  base offset != 0 still works\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
