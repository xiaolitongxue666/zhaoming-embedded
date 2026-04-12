/**
 * @file     motor.c
 * @brief    电机模块实现 - motor.h + motor.c = 一个完整的"类"
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP08 - 你用C手搓了一个class
 *
 * 电机模块和LED模块结构完全一样：
 *   - static辅助函数（private方法）
 *   - 公共函数（public方法）
 *   - init/deinit（构造/析构）
 *   - 前缀 motor_（类名）
 *
 * 两个模块和平共处，函数名永远不会冲突。
 *
 * C++ 写法：  Motor motor(pin);
 *             motor.start();
 *             motor.setSpeed(80);
 *
 * C   写法：  Motor_t motor;
 *             motor_init(&motor, pin);
 *             motor_start(&motor);
 *             motor_set_speed(&motor, 80);
 *
 * 语法不同，思想完全一样。
 */

#include "motor.h"
#include <stdio.h>

/* ============================================================
 *  static辅助函数（= C++的private方法）
 * ============================================================ */

/**
 * @brief  统一更新电机硬件状态
 * @note   static = private，外部调不到
 *
 * 真实硬件上，这里会操作PWM控制器。
 * PC模拟环境下用GPIO高低电平代替：高 = 转，低 = 停。
 */
static void update_hardware(Motor_t *me)
{
    platform_gpio_write(me->pin, me->is_running);
}

/**
 * @brief  检查速度值是否合法
 * @return true=合法(0~MOTOR_SPEED_MAX), false=超范围
 */
static bool is_speed_valid(uint8_t speed)
{
    return (speed <= MOTOR_SPEED_MAX);
}

/**
 * @brief  检查引脚号是否合法
 * @return true=合法(0~MOTOR_PIN_MAX), false=超范围
 */
static bool is_pin_valid(uint8_t pin)
{
    return (pin <= MOTOR_PIN_MAX);
}

/* ============================================================
 *  公共函数实现（= C++的public方法）
 * ============================================================ */

/* ---- 构造 / 析构 ---- */

int motor_init(Motor_t *me, uint8_t pin)
{
    /* 第一件事：参数验证 */
    if (me == NULL) {
        return -1;
    }

    if (!is_pin_valid(pin)) {
        printf("  [Motor] Error: Pin %d out of range (0~%d)\n", pin, MOTOR_PIN_MAX);
        return -2;
    }

    /* 第二件事：硬件配置 */
    platform_gpio_init(pin, GPIO_MODE_OUTPUT);

    /* 第三件事：设置默认状态 */
    me->pin = pin;
    me->speed = 0;
    me->is_running = false;
    me->initialized = true;    /* 标记：已初始化 */

    /* 确保上电后电机是停的 */
    update_hardware(me);

    printf("  [Motor] Pin %d initialized (open for business)\n", pin);
    return 0;
}

int motor_deinit(Motor_t *me)
{
    if (me == NULL) {
        return -1;
    }

    /* 第一步：停止电机（确保安全） */
    me->is_running = false;
    me->speed = 0;
    update_hardware(me);

    /* 第二步：释放GPIO资源 */
    platform_gpio_deinit(me->pin);

    /* 清除所有状态 */
    me->initialized = false;   /* 标记：已反初始化 */

    printf("  [Motor] Pin %d released (closed)\n", me->pin);
    return 0;
}

/* ---- 启动 / 停止 ---- */

int motor_start(Motor_t *me)
{
    if (me == NULL) {
        return -1;
    }

    if (!me->initialized) {
        printf("  [Motor] Error: not initialized, call motor_init() first!\n");
        return -3;
    }

    me->is_running = true;
    update_hardware(me);

    printf("  [Motor] Pin %d Motor started, speed %d%%\n", me->pin, me->speed);
    return 0;
}

int motor_stop(Motor_t *me)
{
    if (me == NULL) {
        return -1;
    }

    if (!me->initialized) {
        printf("  [Motor] Error: not initialized, call motor_init() first!\n");
        return -3;
    }

    me->is_running = false;
    update_hardware(me);

    printf("  [Motor] Pin %d Motor stopped\n", me->pin);
    return 0;
}

/* ---- 速度 ---- */

int motor_set_speed(Motor_t *me, uint8_t speed)
{
    if (me == NULL) {
        return -1;
    }

    if (!me->initialized) {
        printf("  [Motor] Error: not initialized, call motor_init() first!\n");
        return -3;
    }

    if (!is_speed_valid(speed)) {
        printf("  [Motor] Error: speed %d out of range (0~%d)\n", speed, MOTOR_SPEED_MAX);
        return -2;
    }

    me->speed = speed;

    /* 速度为0时自动停止，大于0时如果已启动则保持 */
    if (speed == 0) {
        me->is_running = false;
        update_hardware(me);
    }

    printf("  [Motor] Pin %d speed set to %d%%\n", me->pin, speed);
    return 0;
}

/* ---- 状态查询 ---- */

int motor_get_state(const Motor_t *me, bool *is_running, uint8_t *speed)
{
    if (me == NULL) {
        return -1;
    }

    if (is_running != NULL) {
        *is_running = me->is_running;
    }

    if (speed != NULL) {
        *speed = me->speed;
    }

    return 0;
}
