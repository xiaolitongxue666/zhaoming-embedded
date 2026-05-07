/* SPDX-License-Identifier: MIT */
/*
 * pwm_board.c - STM32 board-level PWM driver implementation (HAL TIM based).
 *
 * STM32 真机 pwm 子类: 把 platform_pwm 的 enable / disable / set_duty 三个
 * 接口翻译成 HAL_TIM_PWM_Start / Stop + __HAL_TIM_SET_COMPARE.
 *
 * 教学完整版只演示 ops 表注册 + duty 折算, 真机引脚 / 时钟 / TIM 周期都从
 * STM32CubeMX 生成的 Core/Src/main.c 完成 (MX_TIMx_Init 调过之后 htimx 的
 * Period / Prescaler 已经写进寄存器).
 *
 * 通道编号约定: driver / 应用层看到的是 channel 0..N-1 这种线性编号,
 * 跟 platform_pwm.h 接口一致. 这一份做 channel -> (htim, TIM_CHANNEL_x)
 * 的解码. 跨开发板换 TIM / 通道时只改 _pwm_chan_table.
 */

#include <stddef.h>

#include "platform/platform_module_export.h"
#include "platform/platform_pwm.h"
#include "stm32f4xx_hal.h"

/* CubeMX 生成的 TIM 句柄, 真机 main 侧 MX_TIMx_Init 之后填进去, 这里 extern. */
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

/* channel 0..N-1 -> (TIM 句柄, TIM_CHANNEL_x) 解码表. 同款表把 IO expander
 * channel 跟 TIM 通道映射拉成一处, 换板只改这一处. */
struct pwm_chan_map {
	TIM_HandleTypeDef *htim;
	uint32_t           channel;
};

static const struct pwm_chan_map _pwm_chan_table[] = {
	{ &htim3, TIM_CHANNEL_1 },     /* channel 0: TIM3_CH1 */
	{ &htim3, TIM_CHANNEL_2 },     /* channel 1: TIM3_CH2 */
	{ &htim4, TIM_CHANNEL_1 },     /* channel 2: TIM4_CH1 */
	{ &htim4, TIM_CHANNEL_2 },     /* channel 3: TIM4_CH2 */
};
#define PWM_CHAN_NUM     ((int32_t)(sizeof(_pwm_chan_table) / sizeof(_pwm_chan_table[0])))

static platform_err_t _stm32_pwm_enable(int32_t channel)
{
	platform_err_t ret = PLATFORM_EINVAL;

	if ((channel < 0) || (channel >= PWM_CHAN_NUM)) {
		goto exit;
	}

	(void)HAL_TIM_PWM_Start(_pwm_chan_table[channel].htim,
	                        _pwm_chan_table[channel].channel);
	ret = PLATFORM_EOK;

exit:
	return ret;
}

static platform_err_t _stm32_pwm_disable(int32_t channel)
{
	platform_err_t ret = PLATFORM_EINVAL;

	if ((channel < 0) || (channel >= PWM_CHAN_NUM)) {
		goto exit;
	}

	(void)HAL_TIM_PWM_Stop(_pwm_chan_table[channel].htim,
	                       _pwm_chan_table[channel].channel);
	ret = PLATFORM_EOK;

exit:
	return ret;
}

static platform_err_t _stm32_pwm_set_duty(int32_t channel, uint8_t duty)
{
	platform_err_t  ret = PLATFORM_EINVAL;
	uint32_t        arr;
	uint32_t        ccr;

	if ((channel < 0) || (channel >= PWM_CHAN_NUM)) {
		goto exit;
	}

	/* duty 0-255 (driver / 应用层契约) -> CCR (TIM 寄存器值).
	 * ARR 是 CubeMX 生成的 Period 寄存器, MX_TIMx_Init 之后已经写进去, 这里
	 * 运行期读出来按比例算 CCR, 跟 ARR 解耦, 换 TIM 周期不用改 driver. */
	arr = __HAL_TIM_GET_AUTORELOAD(_pwm_chan_table[channel].htim);
	ccr = ((uint32_t)duty * arr) / 255u;

	__HAL_TIM_SET_COMPARE(_pwm_chan_table[channel].htim,
	                      _pwm_chan_table[channel].channel, ccr);
	ret = PLATFORM_EOK;

exit:
	return ret;
}

static const struct platform_pwm_ops _stm32_pwm_ops = {
	.enable   = _stm32_pwm_enable,
	.disable  = _stm32_pwm_disable,
	.set_duty = _stm32_pwm_set_duty,
};

static void _pwm_board_init(void)
{
	(void)platform_pwm_register(&_stm32_pwm_ops);
}
INIT_BOARD_EXPORT(_pwm_board_init);
