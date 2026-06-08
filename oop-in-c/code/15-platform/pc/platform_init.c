/* SPDX-License-Identifier: MIT */
/*
 * platform_init.c - Platform 层开机入口 (PC 端)
 *
 * 本文件只做编排: 把 PC 后端的 ops 表注册进 dispatcher, 不写 LED、不碰
 * 寄存器. 详见 pc/README.md § platform_init / Dispatcher.
 *
 * 注册链 (PWM 为例):
 *   platform_init()
 *     → platform_pc_pwm_init()          [platform_pwm_pc.c]
 *       → platform_pwm_register(&pc_pwm_ops)
 *         → _g_ops = ...                [../platform/platform_pwm.c dispatcher]
 *
 * I2C 同理: platform_pc_i2c_init() → platform_i2c_bus_register().
 *
 * 必须在 led_board_init() 之前调用: board 要 platform_i2c_bus_get(), driver
 * 要 platform_pwm_enable(); 未注册则 _g_ops / _g_bus 为 NULL 会失败.
 *
 * 层号 vs 顺序: Platform 是栈底 (第 4 层), 但开机最先 init —— 先通
 * "Platform 管线", 再配 Board 接线, 最后 App 跑业务.
 *
 * GPIO 不在此注册: common/platform_pc.c 仍是直接函数, 无 ops+dispatcher.
 *
 * 真机对应: platform/arch/<mcu>/{pin,pwm,i2c}_board.c 里的
 * platform_hw_xxx_init(), register 风格相同, ops 表换成 HAL/SDK 实现.
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
