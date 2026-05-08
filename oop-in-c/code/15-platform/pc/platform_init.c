/* SPDX-License-Identifier: MIT */
/*
 * platform_init.c - platform 层 ops 表注册 (PC 端)
 *
 * 把 PC 后端的 platform_pwm / platform_i2c ops 表注册进 dispatcher,
 * 给所有外设共用. 在 main 里 led_board_init() 之前调一次.
 *
 * 真实板子上对应的是 STM32 / NXP 的 platform/arch/<mcu>/{pin,pwm,i2c}_
 * board.c 里的 platform_hw_xxx_init 三函数. 三处都是同一个 register 风格
 * 注册, 注册的 ops 表不同而已.
 */

#include "platform_init.h"

/* PC 端 platform_pwm / platform_i2c 注册函数, 实现分别在
 * platform_pwm_pc.c / platform_i2c_pc.c. */
extern void platform_pc_pwm_init(void);
extern void platform_pc_i2c_init(void);

int platform_init(void)
{
	platform_pc_pwm_init();
	platform_pc_i2c_init();
	return 0;
}
