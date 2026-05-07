/* SPDX-License-Identifier: MIT */
/*
 * main.c - Linux 用户态主程序 (3 子类混搭演示).
 *
 * 启动流程:
 *   1. main 调 environment_init() 装配 4 颗 LED 实例
 *      (gpiod_chip_open + gpiod_line_request_output + sysfs PWM export +
 *       open i2c-N + ioctl(I2C_SLAVE))
 *   2. 主循环跑 3 圈, 每圈走一遍 4 个句柄: on -> set_brightness -> sleep -> off
 *   3. environment_exit() 释放 line / fd / chip
 *
 * 应用层调用 led_on / led_off / led_set_brightness 一字不知底下是 libgpiod /
 * sysfs PWM / i2c-dev. OOP 抽象 (struct led_base + 多子类多态 + 设备句柄统
 * 一导出) 在 Linux 用户态仍然有用 -- 它解决"应用层不知道下层硬件细节". 但
 * platform 抽象层只在没有现成设备模型的环境才有价值, 在 Linux 上是反工程,
 * 这个工程已经把它去掉了.
 */

#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "drivers/led/led_base.h"
#include "environment_cfg/environment_export.h"

#define DEMO_ROUNDS    3

static volatile int _g_running = 1;

static void _on_sigint(int sig)
{
	(void)sig;
	_g_running = 0;
}

int main(void)
{
	struct led_base *all[4];
	int              ret;
	int              round;
	int              i;

	signal(SIGINT,  _on_sigint);
	signal(SIGTERM, _on_sigint);

	printf("=========================================\n");
	printf("  linux_full: 4 LED demo\n");
	printf("  GPIO + PWM + I2C 三种子类混搭\n");
	printf("=========================================\n");

	ret = environment_init();
	if (ret != 0) {
		fprintf(stderr, "[main] environment_init failed, exit.\n");
		return 1;
	}

	all[0] = led_status;
	all[1] = led_dimmer;
	all[2] = led_panel;
	all[3] = led_alarm;

	for (round = 0; (round < DEMO_ROUNDS) && _g_running; round++) {
		printf("\n--- round %d ---\n", round);
		for (i = 0; (i < 4) && _g_running; i++) {
			printf("[app] led_on(%s)\n", all[i]->name);
			(void)led_on(all[i]);
			(void)led_set_brightness(all[i], 128);
			sleep(1);
			printf("[app] led_off(%s)\n", all[i]->name);
			(void)led_off(all[i]);
		}
	}

	printf("\n[main] cleaning up\n");
	for (i = 0; i < 4; i++) {
		(void)led_off(all[i]);
	}
	environment_exit();

	printf("=========================================\n");
	printf("  done\n");
	printf("=========================================\n");
	return 0;
}
