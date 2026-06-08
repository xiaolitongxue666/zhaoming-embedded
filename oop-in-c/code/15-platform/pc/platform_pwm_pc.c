/* SPDX-License-Identifier: MIT */
/*
 * platform_pwm_pc.c - PC 端 Platform 后端 + 注册 (ch15 教学).
 *
 * 角色 = 真机 platform/arch/stm32/pwm_board.c 或 arch/nxp/pwm_board.c:
 * 实现 ops 表 + 启动期 register 进 dispatcher. 区别只在 STM32 写 TIM 比较
 * 寄存器, 这里 printf. 同一份 led_pwm.c 只调 platform_pwm_enable(), 不
 * 知道下面是 PC 还是 HAL.
 *
 * 注册链:
 *   platform_init() → platform_pc_pwm_init() → platform_pwm_register(&pc_pwm_ops)
 *   → ../platform/platform_pwm.c 的 _g_ops
 *
 * Dispatcher / 四层架构说明见 15-platform/pc/README.md.
 */

#include "platform/platform_pwm.h"
#include <stdio.h>

static int _pc_pwm_enable(int32_t channel)
{
	printf("  [PWM] ch%d enable\n", channel);
	return 0;
}

static int _pc_pwm_disable(int32_t channel)
{
	printf("  [PWM] ch%d disable\n", channel);
	return 0;
}

static int _pc_pwm_set_duty(int32_t channel, uint8_t duty)
{
	printf("  [PWM] ch%d duty=%u/255\n", channel, (unsigned)duty);
	return 0;
}

static const struct platform_pwm_ops pc_pwm_ops = {
	.enable   = _pc_pwm_enable,
	.disable  = _pc_pwm_disable,
	.set_duty = _pc_pwm_set_duty,
};

/* 启动期注册 PC pwm ops. platform_init 调一次. */
void platform_pc_pwm_init(void)
{
	(void)platform_pwm_register(&pc_pwm_ops);
}
