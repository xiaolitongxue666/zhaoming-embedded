/* SPDX-License-Identifier: MIT */
/*
 * pwm_board_pc.c - PC mock board-level PWM driver.
 *
 * 用 printf 假装 PWM 控制器. 启动期通过 INIT_BOARD_EXPORT 自动注册到
 * platform_pwm 框架, 跟真机版接口一致, driver 层 / 应用层零改动.
 *
 * duty 0-255 是 driver / 应用层契约 (8 位线性). 显示给人看时折算成百分比
 * 更直观, 这一份做最后的折算.
 */

#include <stdio.h>

#include "platform/platform_module_export.h"
#include "platform/platform_pwm.h"

static platform_err_t _pc_pwm_enable(int32_t channel)
{
	printf("    [PWM] ch%d enable\n", (int)channel);
	return PLATFORM_EOK;
}

static platform_err_t _pc_pwm_disable(int32_t channel)
{
	printf("    [PWM] ch%d disable\n", (int)channel);
	return PLATFORM_EOK;
}

static platform_err_t _pc_pwm_set_duty(int32_t channel, uint8_t duty)
{
	unsigned percent = ((unsigned)duty * 100u + 127u) / 255u;
	printf("    [PWM] ch%d duty=%u%% (raw=%u/255)\n",
	       (int)channel, percent, (unsigned)duty);
	return PLATFORM_EOK;
}

static const struct platform_pwm_ops _pc_pwm_ops = {
	.enable   = _pc_pwm_enable,
	.disable  = _pc_pwm_disable,
	.set_duty = _pc_pwm_set_duty,
};

static void _pwm_board_init(void)
{
	(void)platform_pwm_register(&_pc_pwm_ops);
}
INIT_BOARD_EXPORT(_pwm_board_init);
