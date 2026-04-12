/**
 * @file     motor.h
 * @brief    电机模块公共接口 - "motor_"前缀 = 另一个类
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP08 - 你用C手搓了一个class
 *
 * EP08用电机模块证明一件事：
 *   LED模块的函数叫 led_init, led_on, led_off；
 *   电机模块的函数叫 motor_init, motor_start, motor_stop；
 *   前缀不同，永远不冲突 —— 前缀就是类名。
 *
 *   以前两个模块都叫 init()、on()、off() → 编译报错"名称冲突"。
 *   加了前缀，编译器高兴，程序员也看得懂。
 *
 *   "沙县小吃"和"兰州拉面"都卖汤，但招牌不同，你不会走错门。
 *
 *   motor.h + motor.c = 一个完整的电机"类"。
 */

#ifndef MOTOR_H
#define MOTOR_H

#include "platform.h"

/* ---- 引脚与速度有效范围 ---- */
#define MOTOR_PIN_MAX      15   /* 引脚号范围：0 ~ 15 */
#define MOTOR_SPEED_MAX    100  /* 速度范围：0 ~ 100  */

/* ---- 数据结构（= C++的成员变量）---- */

/**
 * @brief  电机对象
 *
 * 【重要约定】
 *   请通过 motor_xxx() 函数操作电机，不要直接修改成员！
 */
typedef struct {
    uint8_t  pin;           /* 控制引脚        —— 请勿直接修改 */
    uint8_t  speed;         /* 速度，0~100     —— 请勿直接修改 */
    bool     is_running;    /* 是否正在运行    —— 请勿直接修改 */
    bool     initialized;   /* 是否已初始化    —— 请勿直接修改 */
} Motor_t;

/* ---- 公共函数声明（= C++的public方法）---- */

/**
 * @brief  初始化电机（= 构造函数，开门营业）
 *
 * 做三件事：
 *   1. 参数验证（引脚范围检查）
 *   2. 硬件配置（GPIO初始化为输出模式）
 *   3. 默认状态（停止运行、速度0）
 *
 * @param  me   指向电机对象
 * @param  pin  控制引脚号（0 ~ MOTOR_PIN_MAX）
 * @return 0=成功, -1=参数错误, -2=引脚超范围
 */
int motor_init(Motor_t *me, uint8_t pin);

/**
 * @brief  反初始化电机（= 析构函数，打烊收工）
 *
 * 做两件事：
 *   1. 停止电机（确保硬件安全）
 *   2. 释放GPIO资源
 *
 * @param  me  指向电机对象
 * @return 0=成功, -1=参数错误
 */
int motor_deinit(Motor_t *me);

/**
 * @brief  启动电机
 * @param  me  指向电机对象
 * @return 0=成功, -1=参数错误, -3=未初始化
 */
int motor_start(Motor_t *me);

/**
 * @brief  停止电机
 * @param  me  指向电机对象
 * @return 0=成功, -1=参数错误, -3=未初始化
 */
int motor_stop(Motor_t *me);

/**
 * @brief  设置电机速度
 * @param  me     指向电机对象
 * @param  speed  速度值，0~100
 * @return 0=成功, -1=参数错误, -2=速度超范围, -3=未初始化
 */
int motor_set_speed(Motor_t *me, uint8_t speed);

/**
 * @brief  获取电机当前状态
 * @param  me          指向电机对象
 * @param  is_running  输出：是否运行中（可传NULL）
 * @param  speed       输出：当前速度（可传NULL）
 * @return 0=成功, -1=参数错误
 */
int motor_get_state(const Motor_t *me, bool *is_running, uint8_t *speed);

#endif /* MOTOR_H */
