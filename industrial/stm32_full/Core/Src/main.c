/* SPDX-License-Identifier: MIT */
/*
 * main.c - STM32F407 firmware entry (CubeMX 风格).
 *
 * 真机入口骨架: 上电 -> startup -> Reset_Handler -> SystemInit -> main ->
 *   HAL_Init -> SystemClock_Config -> MX_GPIO_Init -> 调
 *   platform_module_export_exec() 顺序跑 7 级 INIT_xxx_EXPORT 注册项 ->
 *   主循环.
 *
 * 跟 CubeMX 生成的 main.c 对齐: 函数命名、USER CODE 注释段都按 CubeMX 风
 * 写, 方便用户把这一份覆盖回 CubeMX 工程.
 */

#include <stddef.h>
#include <stdint.h>

#include "main.h"

#include "drivers/led/led_base.h"
#include "environment_cfg/environment_export.h"
#include "platform/platform_module_export.h"

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* CubeMX 生成的函数原型 (用户自己 CubeMX 工程提供) */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/* Powered-on busy delay, ~1ms scale @168 MHz */
static void delay_ms(uint32_t ms)
{
	for (volatile uint32_t i = 0; i < ms * 16800UL; i++) {
		;
	}
}

int main(void)
{
	struct led_base *all[4];
	uint32_t         cur = 0;
	uint8_t          dimmer_level = 0;

	/* MCU Configuration */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();

	/* USER CODE BEGIN 2 */

	/* 这一行调用所有 INIT_BOARD/PREV/DEVICE/COMPONENT/ENV/APP/SYSTEM_READY
	 * 注册项: pin_board.c 注册 pin ops, led_cfg.c 装配 4 颗 LED 实例. */
	platform_module_export_exec();

	all[0] = led_status;
	all[1] = led_dimmer;
	all[2] = led_panel;
	all[3] = led_alarm;

	/* USER CODE END 2 */

	while (1) {
		/* 应用层: 流水灯, 同时演示 dimmer 调亮度 */
		for (uint32_t i = 0; i < 4; i++) {
			(void)led_off(all[i]);
		}
		(void)led_on(all[cur]);
		(void)led_set_brightness(all[cur], dimmer_level);

		cur = (cur + 1) % 4;
		dimmer_level += 32;     /* 0, 32, 64, 96, ... 自然回卷 */
		delay_ms(1000);
	}
}

/* CubeMX 生成的占位 (真机移植时把 CubeMX 输出抄回这两个函数) */
void SystemClock_Config(void)
{
	/* USER: 替换为 CubeMX 生成版 */
}

static void MX_GPIO_Init(void)
{
	/* USER: 替换为 CubeMX 生成版 (LED 引脚时钟使能由 pin_board.c 完成) */
}
