/**
  ******************************************************************************
  * @file    v_motor_base.c
  * @brief   垂直电机基类的封装函数实现。每个公开函数体都很短：
  *          先用 platform_assert 校验关键纯虚方法不为 NULL，
  *          再维护 move_state、按需触发回调、把命令派发到 ops 表。
  *
  *          应用层调 v_motor_base_move(motor, UP) 时的完整路径：
  *
  *            v_motor_base_move(me, UP)
  *               │
  *               ├─ platform_assert(me / me->ops / motor_move 都非 NULL)
  *               ├─ 检查方向是否变化，变了就调 status_changed_cb
  *               ├─ 更新 me->move_state = UP
  *               └─ me->ops->motor_move(me, UP)   ← 派发到子类
  *                       │
  *                       └─ v_motor_ifx007t.c::motor_move(...)
  *                              │
  *                              └─ 把命令推进 work_queue 立刻返回
  *                                 （后续在 work_thread 里跑 PWM 逻辑）
  *
  *          纯虚方法 vs 可空虚方法：
  *            motor_stop / motor_move          ← 纯虚（每个子类必须实现）
  *            fault_clear                      ← 可空（无故障锁的子类可不实现）
  *          纯虚方法用 platform_assert 卡死"子类忘了挂钩"的低级 bug；
  *          可空方法用 if (me->ops->xxx) 兜底。
  ******************************************************************************
  */

#include "v_motor_base.h"
#include "platform_assert.h"
#include <string.h>
#include <stddef.h>

/* 基类初始化：把所有字段清零 / NULL 化。子类构造函数
 * 第一件事就是调这个，再设自己的 ops 和私有字段。 */
void v_motor_init(v_motor_base_t *me)
{
    platform_assert(me != NULL);

    me->ops = NULL;
    me->move_state = V_MOTOR_BASE_MOVE_STOP;
    me->v_motor_move_status_changed_cb = NULL;
    me->err_cb = NULL;
}

/* 应用层入口：往 dir 方向运动。
 * 触发顺序：先回调通知"方向变了"，再更新状态，最后派发 ops。
 * 之所以先回调再 dispatch：让上层 UI / 日志有机会先记录"开始运动"，
 * 然后底层 PWM 才真正动起来。
 *
 * motor_move 是纯虚——子类构造函数必须把 ops->motor_move 挂上钩，
 * 否则上电就会在这里 platform_assert 失败。 */
void v_motor_base_move(v_motor_base_t *me,
                       v_motor_base_dir_t dir)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->motor_move != NULL);

    if (me->v_motor_move_status_changed_cb
        && me->move_state != dir)
    {
        me->v_motor_move_status_changed_cb(me, dir);
    }
    me->move_state = dir;
    me->ops->motor_move(me, dir);
}

/* 立即停车。同样先回调再 dispatch。
 * motor_stop 也是纯虚。 */
void v_motor_base_stop(v_motor_base_t *me)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->motor_stop != NULL);

    if (me->v_motor_move_status_changed_cb
        && me->move_state != V_MOTOR_BASE_MOVE_STOP)
    {
        me->v_motor_move_status_changed_cb(
            me, V_MOTOR_BASE_MOVE_STOP);
    }
    me->move_state = V_MOTOR_BASE_MOVE_STOP;
    me->ops->motor_stop(me);
}

v_motor_base_dir_t v_motor_base_status_get(v_motor_base_t *me)
{
    platform_assert(me != NULL);

    return me->move_state;
}

void v_motor_move_status_change_cb_register(
    v_motor_base_t *me,
    v_motor_move_status_changed cb)
{
    platform_assert(me != NULL);

    me->v_motor_move_status_changed_cb = cb;
}

void v_motor_base_fault_cb_register(
    v_motor_base_t *me,
    v_motor_err_cb_t cb)
{
    platform_assert(me != NULL);

    me->err_cb = cb;
}

/* fault_clear 是可空虚方法。子类不实现就 no-op。
 * 这里的 if 兜底让简单子类不需要写空函数占位。
 * 注意：这里不用 platform_assert 卡 fault_clear 不为 NULL，
 * 那样会要求所有子类都得提供（即使根本没有故障锁机制）。 */
void v_motor_base_fault_clear(v_motor_base_t *me)
{
    platform_assert(me != NULL);

    if (me->ops && me->ops->fault_clear)
    {
        me->ops->fault_clear(me);
    }
}
