/* SPDX-License-Identifier: MIT */
/*
 * platform_pwm_pc.c - PC 端 platform_pwm ops 实现 + 注册 (ch15 教学).
 *
 * 角色和 platform/arch/stm32/pin_board.c 里 _stm32_pwm_* 同款, 区别只在:
 * STM32 端 _stm32_pwm_set_duty 调 __HAL_TIM_SET_COMPARE 写真实定时器
 * 比较寄存器, 这里翻译成 stdout printf. 同一份 led_pwm.c 子类源码字节级
 * 不动, 跨平台跑同一个 platform_pwm dispatch.
 *
 * 落地形态: 一张 static const struct platform_pwm_ops 表 + 一个启动期
 * 注册函数 platform_pc_pwm_init, 由 platform_init 调一次.
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
