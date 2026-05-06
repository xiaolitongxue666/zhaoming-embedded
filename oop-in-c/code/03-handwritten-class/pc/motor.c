/* SPDX-License-Identifier: MIT */
/*
 * motor.c - 电机模块实现
 *
 * 和 led.c 是同一个套路。读完 led.c 你扫一眼这个文件就懂，
 * 这就是好命名规范的力量：函数前缀 + init/deinit 生命周期 +
 * static 工具函数。
 *
 * PC 模拟环境用 GPIO 高低电平假装控制电机，真实硬件上
 * pwm_duty 会写到 TIM 通道的 CCRx 寄存器。
 */

#include "motor.h"
#include <stdio.h>

/* ---- file-private 工具函数 ---- */

static void update_hardware(struct motor *me)
{
	bool running = (me->state != MOTOR_STOPPED);
	platform_gpio_write(me->pin, running);
}

static bool pwm_valid(uint8_t pwm_duty)
{
	return pwm_duty <= 100;
}

static bool pin_valid(uint8_t pin)
{
	return pin <= MOTOR_PIN_MAX;
}

/* ---- 生命周期 ---- */

int motor_init(struct motor *me, uint8_t pin)
{
	if (!me)
		return -1;
	if (!pin_valid(pin)) {
		printf("  [MOTOR] Error: pin %u out of range\n",
		       (unsigned)pin);
		return -2;
	}

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);

	me->pin = pin;
	me->pwm_duty = 0;
	me->direction = true;
	me->state = MOTOR_STOPPED;
	me->initialized = true;

	update_hardware(me);

	printf("  [MOTOR] Pin%u initialized\n", (unsigned)pin);
	return 0;
}

int motor_deinit(struct motor *me)
{
	if (!me)
		return -1;

	me->state = MOTOR_STOPPED;
	update_hardware(me);
	platform_gpio_deinit(me->pin);

	me->pwm_duty = 0;
	me->initialized = false;

	printf("  [MOTOR] Pin%u released\n", (unsigned)me->pin);
	return 0;
}

/* ---- 操作 ---- */

int motor_start(struct motor *me)
{
	if (!me)
		return -1;
	if (!me->initialized) {
		printf("  [MOTOR] Error: not initialized\n");
		return -3;
	}

	me->state = me->direction ? MOTOR_FORWARD : MOTOR_REVERSE;
	update_hardware(me);

	printf("  [MOTOR] Pin%u start (%s, duty=%u%%)\n",
	       (unsigned)me->pin,
	       me->direction ? "forward" : "reverse",
	       (unsigned)me->pwm_duty);
	return 0;
}

int motor_stop(struct motor *me)
{
	if (!me)
		return -1;
	if (!me->initialized) {
		printf("  [MOTOR] Error: not initialized\n");
		return -3;
	}

	me->state = MOTOR_STOPPED;
	update_hardware(me);

	printf("  [MOTOR] Pin%u stop\n", (unsigned)me->pin);
	return 0;
}

int motor_set_speed(struct motor *me, uint8_t pwm_duty)
{
	if (!me)
		return -1;
	if (!me->initialized) {
		printf("  [MOTOR] Error: not initialized\n");
		return -3;
	}
	if (!pwm_valid(pwm_duty)) {
		printf("  [MOTOR] Error: duty %u out of range (0~100)\n",
		       (unsigned)pwm_duty);
		return -2;
	}

	me->pwm_duty = pwm_duty;
	if (pwm_duty == 0)
		me->state = MOTOR_STOPPED;
	update_hardware(me);

	printf("  [MOTOR] Pin%u duty set to %u%%\n",
	       (unsigned)me->pin, (unsigned)pwm_duty);
	return 0;
}

int motor_set_direction(struct motor *me, bool forward)
{
	if (!me)
		return -1;
	if (!me->initialized) {
		printf("  [MOTOR] Error: not initialized\n");
		return -3;
	}

	me->direction = forward;
	if (me->state != MOTOR_STOPPED)
		me->state = forward ? MOTOR_FORWARD : MOTOR_REVERSE;

	printf("  [MOTOR] Pin%u direction = %s\n",
	       (unsigned)me->pin, forward ? "forward" : "reverse");
	return 0;
}

/* ---- 查询 ---- */

int motor_get_state(const struct motor *me, uint8_t *state, uint8_t *pwm_duty)
{
	if (!me)
		return -1;

	if (state)
		*state = me->state;
	if (pwm_duty)
		*pwm_duty = me->pwm_duty;

	return 0;
}
