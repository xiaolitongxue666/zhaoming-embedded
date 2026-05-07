/* SPDX-License-Identifier: MIT */
/*
 * main_pc.c - PC mock 入口, 替代 Core/Src/main.c 跑在 PC 上.
 *
 * 验证 stm32_full 整套抽象在 PC 上能 build + run. 所有 GPIO / PWM / I2C
 * 操作走 mock 板级实现打到 stdout, 没有真机时也能看 LED 行为. 这种"真机
 * + PC mock 双模 build"是工业项目里常见做法, 让 OOP 抽象在 PC 上也能
 * 单元测试 / 跑 CI.
 *
 * 真机 main 调 platform_module_export_exec() 跑完所有 INIT_xxx_EXPORT;
 * MOCK 模式下 ctor 已经在 main 之前自动跑过, 这里调 platform_module_export_exec
 * 是 nop. 应用层调用形态完全一致.
 */

#include <stdint.h>
#include <stdio.h>

#include "drivers/led/led_base.h"
#include "environment_cfg/environment_export.h"
#include "platform/platform_module_export.h"

#define ROUNDS    3

int main(void)
{
	struct led_base *all[4];
	uint32_t         round;
	uint32_t         i;

	printf("=========================================\n");
	printf("  stm32_full PC mock: 4 LED demo\n");
	printf("  GPIO + PWM + I2C 三种子类混搭\n");
	printf("=========================================\n");

	/* 真机这一行跑 7 级 INIT_xxx_EXPORT;
	 * MOCK 下 ctor 已经在 main 之前跑过, 这里是 nop. */
	platform_module_export_exec();

	if ((NULL == led_status) || (NULL == led_dimmer) ||
	    (NULL == led_panel)  || (NULL == led_alarm)) {
		printf("[main] LED handles not ready, exit.\n");
		return -1;
	}

	all[0] = led_status;
	all[1] = led_dimmer;
	all[2] = led_panel;
	all[3] = led_alarm;

	for (round = 0; round < ROUNDS; round++) {
		printf("\n--- round %u ---\n", (unsigned)round);
		for (i = 0; i < 4; i++) {
			printf("[app] led_on(%s)\n", all[i]->name);
			(void)led_on(all[i]);
			(void)led_set_brightness(all[i], 128);
			printf("[app] led_off(%s)\n", all[i]->name);
			(void)led_off(all[i]);
		}
	}

	printf("\n=========================================\n");
	printf("  done (%u rounds)\n", (unsigned)ROUNDS);
	printf("=========================================\n");
	return 0;
}
