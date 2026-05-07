/* Minimal syntax stub for fsl_pwm.h */
#ifndef FSL_PWM_H
#define FSL_PWM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct { int dummy; } PWM_Type;
#define PWM1 ((PWM_Type *)0x403E0000UL)

typedef enum {
	kPWM_Module_0 = 0,
	kPWM_Module_1,
} pwm_submodule_t;

typedef enum {
	kPWM_PwmA = 0,
	kPWM_PwmB,
	kPWM_PwmX,
} pwm_channels_t;

typedef enum {
	kPWM_HighTrue = 0,
	kPWM_LowTrue,
} pwm_level_select_t;

typedef enum {
	kPWM_SignedCenterAligned = 0,
	kPWM_SignedEdgeAligned,
} pwm_mode_t;

typedef struct {
	pwm_channels_t     pwmChannel;
	pwm_level_select_t level;
	uint8_t            dutyCyclePercent;
} pwm_signal_param_t;

void PWM_StartTimer(PWM_Type *base, uint32_t mask);
void PWM_StopTimer(PWM_Type *base, uint32_t mask);
void PWM_UpdatePwmDutycycle(PWM_Type *base, pwm_submodule_t sub,
                            pwm_channels_t ch, pwm_mode_t mode, uint8_t pct);
void PWM_SetPwmLdok(PWM_Type *base, uint32_t mask, bool act);

#endif
