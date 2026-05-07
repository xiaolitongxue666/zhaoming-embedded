/* SPDX-License-Identifier: MIT */
/*
 * platform/arch/nxp/pwm_board.c - NXP i.MX RT1170 端 PWM 后端实现.
 *
 * "认识 NXP eFlexPWM 寄存器"的唯一一份文件. 上层 drivers/led/led_pwm 调
 * platform_pwm_xxx ops 表层接口下来, 落到这里之后才碰 PWM_Type * 加
 * PWM_UpdatePwmDutycycle / PWM_SetPwmLdok.
 *
 * 跨 MCU (STM32 -> NXP) 的"换 6 份之一": 本文 + 同目录 pin_board.c +
 * i2c_board.c 三份是这家 MCU 的 platform 实现层.
 *
 * 唯一不同 (对照 stm32/pwm_board.c):
 *   TIM_HandleTypeDef *      ->  PWM_Type *
 *   HAL_TIM_PWM_Start        ->  PWM_StartTimer
 *   __HAL_TIM_SET_COMPARE    ->  PWM_UpdatePwmDutycycle + PWM_SetPwmLdok
 *
 * 工程假设: MCUXpresso IDE 已经初始化 PWM1 sub-module 0 (4 路 PWM),
 * fsl_pwm 头是 SDK builtin.
 */

#include "platform/platform_pwm.h"
#include "fsl_pwm.h"

static int _nxp_pwm_enable(int32_t channel)
{
	(void)channel;
	PWM_StartTimer(PWM1, kPWM_Module_0);
	return 0;
}

static int _nxp_pwm_disable(int32_t channel)
{
	(void)channel;
	PWM_StopTimer(PWM1, kPWM_Module_0);
	return 0;
}

static int _nxp_pwm_set_duty(int32_t channel, uint8_t duty)
{
	uint8_t pct = (uint8_t)((uint32_t)duty * 100U / 255U);

	PWM_UpdatePwmDutycycle(PWM1, kPWM_Module_0,
	                       (pwm_channels_t)channel, kPWM_SignedCenterAligned,
	                       pct);
	PWM_SetPwmLdok(PWM1, (1U << kPWM_Module_0), true);
	return 0;
}

static const struct platform_pwm_ops _nxp_pwm_ops = {
	.enable   = _nxp_pwm_enable,
	.disable  = _nxp_pwm_disable,
	.set_duty = _nxp_pwm_set_duty,
};

/* 启动期由 board_init 调一次. 调用顺序: pin -> pwm -> i2c. */
void platform_hw_pwm_init(void)
{
	(void)platform_pwm_register(&_nxp_pwm_ops);
}
