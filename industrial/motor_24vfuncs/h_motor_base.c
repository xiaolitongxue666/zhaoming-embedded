/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    h_motor_base.c
  * @brief   水平电机基类的封装函数实现。每个公开函数体几乎都是一行：
  *
  *              me->ops->xxx(me, ...);
  *
  *          这就是 ops 表派发的全部代码。基类层不关心子类是 ACM 伺服
  *          电机还是步进电机，只关心"有 ops 表能派发就行"。
  *
  *          纯虚 vs 可空虚方法：
  *            - 纯虚（每个子类必须实现）：上半段所有 motor_stop / start /
  *              move_xxx / velocity_set / acce_dece_set / status_get /
  *              cur_pos_get / enable / home / home_set / positive_limit /
  *              fault_clear / manual_mode 这一组核心动作。
  *              在派发前用 platform_assert 校验 me / me->ops / 函数指针
  *              都不为 NULL，子类忘记挂 ops 上电就会立刻报死。
  *            - 可空（subclass 可以不实现）：下半段 negative_limit /
  *              following_err_xxx / encoder_ratio_set / i2t_xxx 这一组。
  *              老固件子类如果没有这些能力，留 NULL 即可，基类用
  *              if (me->ops->xxx) 兜底成 no-op。
  *
  *          status_get_sync 单独走完整 NULL 校验然后返回 false：
  *          它会被状态机关键路径调到，必须不会在异常对象状态下崩。
  ******************************************************************************
  */

#include "h_motor_base.h"
#include "platform_assert.h"
#include <string.h>
#include <stddef.h>

/* 把基类对象清零。子类构造函数第一件事调这个，再设自己的字段。 */
void h_motor_base_init(struct h_motor_base *me)
{
    platform_assert(me != NULL);

    memset(me, 0, sizeof(struct h_motor_base));
}

/* ────────────────── 纯虚方法（必须挂 ops） ──────────────────
 * 下面这一组的 platform_assert 是给子类作者看的：
 * 你必须在子类构造函数里把这些函数指针挂上钩，否则上电
 * 调到这里就会立刻 assert fail，把"子类忘了实现"这种低级
 * bug 提前暴露。 */

void h_motor_base_stop(struct h_motor_base *me)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->motor_stop != NULL);

    me->ops->motor_stop(me);
}

void h_motor_base_start(struct h_motor_base *me)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->motor_start != NULL);

    me->ops->motor_start(me);
}

void h_motor_base_move_velocity_mode(struct h_motor_base *me,
                                     h_motor_base_move_dir_t dir)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->move_velocity_mode != NULL);

    me->ops->move_velocity_mode(me, dir);
}

void h_motor_base_move_relative_mode(struct h_motor_base *me,
                                     double move_mm)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->move_relative_mode != NULL);

    me->ops->move_relative_mode(me, move_mm);
}

void h_motor_base_move_absolute_mode(struct h_motor_base *me,
                                     double pos_mm)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->move_absolute_mode != NULL);

    me->ops->move_absolute_mode(me, pos_mm);
}

void h_motor_base_velocity_set(struct h_motor_base *me, int32_t velocity)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->velocity_set != NULL);

    me->ops->velocity_set(me, velocity);
}

void h_motor_base_acce_dece_set(struct h_motor_base *me,
                                uint32_t acce, uint32_t dece)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->acce_dece_set != NULL);

    me->ops->acce_dece_set(me, acce, dece);
}

void h_motor_base_manual_mode(struct h_motor_base *me, bool is_manual)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->manual_mode != NULL);

    me->ops->manual_mode(me, is_manual);
}

void h_motor_base_status_get(struct h_motor_base *me)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->status_get != NULL);

    me->ops->status_get(me);
}

/* status_get_sync 是状态机关键路径上的"必须立刻拿到"接口。
 * 走完整 NULL 校验然后返回 false，让上层有机会按"读不到"
 * 分支降级处理；不像其他纯虚方法那样直接 assert fail。 */
bool h_motor_base_status_get_sync(struct h_motor_base *me,
                                   h_motor_base_status_t *status)
{
    if (me == NULL || me->ops == NULL ||
        me->ops->status_get_sync == NULL || status == NULL)
    {
        return false;
    }
    return me->ops->status_get_sync(me, status);
}

void h_motor_base_status_get_cb_register(struct h_motor_base *me,
                                         status_cb_t cb)
{
    platform_assert(me != NULL);

    me->status_cb = cb;
}

void h_motor_base_cur_pos_get(struct h_motor_base *me)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->cur_pos_get != NULL);

    me->ops->cur_pos_get(me);
}

void h_motor_base_cur_pos_get_cb_register(struct h_motor_base *me,
                                          cur_pos_cb_t cb)
{
    platform_assert(me != NULL);

    me->cur_pos_cb = cb;
}

void h_motor_base_enable(struct h_motor_base *me, bool enable)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->enable != NULL);

    me->ops->enable(me, enable);
}

void h_motor_base_fault_cb_register(struct h_motor_base *me,
                                    err_cb_t cb)
{
    platform_assert(me != NULL);

    me->err_cb = cb;
}

void h_motor_base_home(struct h_motor_base *me)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->home != NULL);

    me->ops->home(me);
}

void h_motor_base_home_set(struct h_motor_base *me, double offset_mm)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->home_set != NULL);

    me->ops->home_set(me, offset_mm);
}

void h_motor_base_positive_limit(struct h_motor_base *me,
                                 uint32_t limit_mm)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->positive_limit != NULL);

    me->ops->positive_limit(me, limit_mm);
}

void h_motor_base_fault_clear(struct h_motor_base *me)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->fault_clear != NULL);

    me->ops->fault_clear(me);
}


/* ────────────────── 可空虚方法（NULL 兜底） ─────────────────
 * 下面这一组允许子类不挂钩。挂上了正常派发，没挂就 no-op。
 * 这种模式让简单子类不必写"空实现"占位，同时保持应用层
 * 调用代码一致——具体子类支不支持这能力由它自己决定。 */

void h_motor_base_negative_limit(struct h_motor_base *me,
                                 int32_t limit_mm)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);

    if (me->ops->negative_limit)
    {
        me->ops->negative_limit(me, limit_mm);
    }
}

void h_motor_base_following_error_enbale(struct h_motor_base *me,
                                         bool enable)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);

    if (me->ops->following_err_enable)
    {
        me->ops->following_err_enable(me, enable);
    }
}

void h_motor_base_following_err_limit_set(struct h_motor_base *me,
                                           double limit_mm)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);

    if (me->ops->following_err_limit_set)
    {
        me->ops->following_err_limit_set(me, limit_mm);
    }
}

void h_motor_base_encoder_ratio_set(struct h_motor_base *me,
                                     double cnt_per_mm)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);

    if (me->ops->encoder_ratio_set)
    {
        me->ops->encoder_ratio_set(me, cnt_per_mm);
    }
}

void h_motor_base_i2t_fault_enable(struct h_motor_base *me,
                                    bool enable)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);

    if (me->ops->i2t_fault_enable)
    {
        me->ops->i2t_fault_enable(me, enable);
    }
}

void h_motor_base_i2t_peak_current_set(
    struct h_motor_base *me, uint16_t current_001a)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);

    if (me->ops->i2t_peak_current_set)
    {
        me->ops->i2t_peak_current_set(me, current_001a);
    }
}

void h_motor_base_i2t_continuous_current_set(
    struct h_motor_base *me, uint16_t current_001a)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);

    if (me->ops->i2t_continuous_current_set)
    {
        me->ops->i2t_continuous_current_set(me, current_001a);
    }
}

void h_motor_base_i2t_peak_time_set(
    struct h_motor_base *me, uint16_t time_ms)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);

    if (me->ops->i2t_peak_time_set)
    {
        me->ops->i2t_peak_time_set(me, time_ms);
    }
}
