/* SPDX-License-Identifier: MIT */
/*
 * v_motor_ifx007t.h - IFX007T H 桥子类（垂直直流电机）。
 *
 * 接 v_motor_base 的 PWM 调速实现。两路 PWM 分别驱动 H 桥两个高边臂，
 * 加 sleep1/sleep2 两根 INH 引脚控制 IC 上电/休眠，再加上下两个减速区
 * 限位开关 + 一路 IS 引脚电流采样。
 *
 * 继承关系：
 *     struct v_motor_ifx007t.base -> struct v_motor_base
 *
 * ops 派发：
 *     motor_move / motor_stop / fault_clear  ──►  内部工作线程
 *     工作线程消费命令队列，串行执行硬件操作；保证 ops 接口立刻
 *     返回（非阻塞），软启动 / 加减速 / 故障检测全部异步运行。
 */

#ifndef __V_MOTOR_IFX007T_H
#define __V_MOTOR_IFX007T_H

#include "v_motor_base.h"
#include "platform_pwm.h"
#include "cmsis_os2.h"
#include <stdint.h>
#include <stdbool.h>

/* IS 故障检测要走 ADC 通道。这里只用前向声明，
 * 让本头文件不强制依赖具体 ADC 驱动接口；上层把已构造好的
 * struct adc_channel_base 指针传进来即可。 */
struct adc_channel_base;

/* 构造参数：六路 GPIO 名 + 两路 PWM 设备名/通道 + 一路 IS ADC 通道。 */
struct v_motor_ifx007t_init_param {
	/* H 桥两个 PWM 输出（分别驱动 ctrl_1 / ctrl_2 高边臂）。
	 * 同一个 PWM 设备的两个通道、或两个独立 PWM 设备都可以，
	 * 由 board 层 PWM 注册决定。 */
	const char *ctrl_1_pwm_dev_name;
	int32_t     ctrl_1_pwm_channel;
	const char *ctrl_2_pwm_dev_name;
	int32_t     ctrl_2_pwm_channel;

	/* IFX007T 的 INH（休眠/使能）引脚。两片 IC 各一根。 */
	const char *sleep1_pin_name;
	const char *sleep2_pin_name;

	/* 上下减速区限位开关引脚。命中后驱动会自动从全速降回起步速度，
	 * 实现"快进 + 慢爬到位"的两段式行程。 */
	const char *top_dece_pin_name;
	const char *bottom_dece_pin_name;

	/* 电流采样通道。NULL 表示不接 IS 检测，驱动会自动跳过过流保护。 */
	struct adc_channel_base *is_adc_channel;
};

/* 子类对象。base 必须放第一字段，向上转型零代价（见 ch12）。 */
struct v_motor_ifx007t {
	struct v_motor_base base;

	/* 硬件句柄 */
	platform_device_pwm_t ctrl_1_pwm_dev;
	platform_device_pwm_t ctrl_2_pwm_dev;
	int32_t ctrl_1_pwm_channel;
	int32_t ctrl_2_pwm_channel;
	int32_t sleep1_pin;
	int32_t sleep2_pin;
	int32_t top_dece_pin;
	int32_t bottom_dece_pin;

	/* 周期工作定时器（10ms 一跳，跑加减速 + 三层故障检测）。 */
	osTimerId_t work_timer;

	/* 工作线程私有方向。仅工作线程读写，不需要锁。 */
	enum v_motor_base_dir work_dir;

	/* 当前 PWM 占空计数（单位与 PWM_PERIOD 同步）。 */
	uint32_t pwm_pulse;

	/* 阶段二：起步占空 -> 满速的线性加速。 */
	uint32_t current_min_pulse;     /* 起步占空对应的 pulse 计数 */
	uint32_t current_step_size;     /* 每 10ms 的递增量 */

	/* 阶段一：5% -> 起步占空的软启动。避免上电瞬间冲击电流。 */
	uint32_t soft_start_min_pulse;  /* 5% 对应的 pulse 计数 */
	uint32_t soft_start_step_size;  /* 每 10ms 的递增量 */

	/* IS 电流采样的 EMA 滤波 + 故障确认状态。 */
	struct adc_channel_base *is_adc_channel;
	float    is_avg_mv;            /* 平滑后的电流采样电压(mV) */
	uint32_t is_fault_count;       /* 连续超阈次数（10 次 = 100ms） */
	uint32_t is_print_count;       /* 调试打印分频计数 */
	bool     is_hw_error;          /* 任何一类故障触发后置 true */

	/* 减速区限位长按超时计数（10ms 一拍）。位 5s 后认为机械卡住。 */
	uint32_t dece_timeout_count;

	/* 整体运动总时长超时计数（10ms 一拍）。位 30s 后强制停车。 */
	uint32_t motion_timeout_count;

	/* 工作线程 + 命令队列。所有 ops 调用都把命令丢进队列就返回，
	 * 真正的硬件操作（串行化的 PWM 写、osDelay、阻塞读）都在
	 * work_thread 里跑，不卡上层调用方。 */
	osThreadId_t       work_thread;
	osMessageQueueId_t work_queue;
};

/* 构造函数。注册 ops、查表三组 GPIO + 两路 PWM、初始化故障检测状态、
 * 创建工作线程 / 命令队列 / 周期定时器。 */
void v_motor_ifx007t_init(struct v_motor_ifx007t *me,
                          struct v_motor_ifx007t_init_param *init_param);

#endif /* __V_MOTOR_IFX007T_H */
