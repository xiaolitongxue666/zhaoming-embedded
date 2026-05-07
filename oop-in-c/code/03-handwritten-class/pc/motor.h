/* SPDX-License-Identifier: MIT */
/*
 * motor.h - 电机模块对外接口
 *
 * 见书 ch03 § 3.1-3.2 名称冲突 + 沙县小吃和兰州拉面。
 *
 * 和 led 模块同一个套路：
 *   - 文件命名：motor.h + motor.c
 *   - 类型命名：struct motor
 *   - 函数前缀：motor_xxx          <- 关键: 不加前缀就 motor.c 写
 *                                       int init() / led.c 写
 *                                       int init(), 链接器立刻报
 *                                       multiple definition of 'init'
 *   - 生命周期：motor_init / motor_deinit
 *
 * 字段不一样，因为电机的状态量本来就和 LED 不一样：
 *   pin       控制方向引脚
 *   pwm_duty  PWM 占空比 0~100（控制速度）
 *   direction 旋转方向 (false=反, true=正)
 *   state     运行状态 (0=停, 1=正转, 2=反转)
 *
 * 见书 § 3.7.7 为什么 motor 多了 direction 和 state: 字段不是越多
 * 越好, 跟 motor 这个对象本身的属性走。"上次启动时间"/"累计运行小时"
 * 这种是另一个层次的数据 (运维监控), 属于另一个模块的职责, 不该
 * 塞进 struct motor。ch04 数据归位会展开。
 */

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
