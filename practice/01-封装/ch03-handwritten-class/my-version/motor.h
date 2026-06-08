
#ifndef MOTOR_H
#define MOTOR_H

#include "platform.h"

#define MOTOR_PIN_MAX  31

enum motor_state {
	MOTOR_STOPPED = 0,
	MOTOR_FORWARD = 1,
	MOTOR_REVERSE = 2,
};

struct motor {
	uint8_t pin;            /* 控制引脚 */
	uint8_t pwm_duty;       /* PWM 占空比 0~100 */
	bool    direction;      /* false = 反, true = 正 */
	uint8_t state;          /* enum motor_state */
	bool    initialized;
};

/* 生命周期 */
int motor_init(struct motor *me, uint8_t pin);
int motor_deinit(struct motor *me);

/* 操作 */
int motor_start(struct motor *me);
int motor_stop(struct motor *me);
int motor_set_speed(struct motor *me, uint8_t pwm_duty);
int motor_set_direction(struct motor *me, bool forward);

/* 查询 */
int motor_get_state(const struct motor *me, uint8_t *state, uint8_t *pwm_duty);

#endif /* MOTOR_H */
