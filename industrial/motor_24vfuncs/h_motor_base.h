/**
  ******************************************************************************
  * @file    h_motor_base.h
  * @brief   水平电机基类（伺服电机抽象）。24 个虚方法 + 3 个回调，
  *          覆盖工业伺服电机的全部能力域。
  *
  *          这是全书最大的 ops 表。每个虚方法都对应一个真实工程问题
  *          （详见 ch20 § 20.1.1 的"24 个虚方法是怎么长出来的"）。
  *          一对照 v_motor_base 的 3 个虚方法，能直观感受到"接口规模
  *          严格匹配硬件能力"——伺服电机闭环、可编程、有 I²t 保护，
  *          所以 24 个方法都用得上；直流电机能力域窄，3 个方法就够了。
  *
  *          应用层只调本头文件下半段声明的 h_motor_base_xxx 封装函数，
  *          不直接访问 ops 字段。基类封装函数内部走
  *              me->ops->xxx(me, ...)
  *          派发到具体子类（h_motor_acm 等）。
  ******************************************************************************
  */

#ifndef __H_MOTOR_H
#define __H_MOTOR_H

#include <stdbool.h>
#include <stdint.h>

struct h_motor_base;

/* 运动方向。1 / -1 让上层应用计算位移时可以直接乘。 */
typedef enum
{
    H_MOTOR_MOVE_POSITIVE = 1,
    H_MOTOR_MOVE_NEGATIVE = -1,
} h_motor_base_move_dir_t;

/* 运动状态聚合体。一次性把驱动内部能观察到的全部布尔状态打包，
 * 通过 status_get_sync 一次性吐给应用层。
 *
 * 字段含义：
 *   is_in_motion         电机当前是否在运动（任何模式都算）
 *   is_homing_state      正在执行回零序列
 *   is_homing_success    上一次回零成功完成
 *   is_homing_error      上一次回零异常退出
 *   is_commnucation_err  通信链路异常（保留拼写以兼容已有调用方）
 *   is_hw_error          硬件层故障锁（过流 / 过温等）
 */
typedef struct
{
    bool is_in_motion;
    bool is_homing_state;
    bool is_homing_success;
    bool is_homing_error;
    bool is_commnucation_err;
    bool is_hw_error;
} h_motor_base_status_t;

/* 24 个虚方法的 ops 表。按业务分组：
 *
 *   ── 启停 ────────────────────────────────────────────
 *     motor_stop / motor_start / enable
 *
 *   ── 三种运动模式（持续 / 相对 / 绝对） ───────────────
 *     move_velocity_mode / move_relative_mode /
 *     move_absolute_mode
 *
 *   ── 运动参数动态调节 ────────────────────────────────
 *     velocity_set / acce_dece_set
 *
 *   ── 状态查询（异步触发 + 同步阻塞两种） ──────────────
 *     status_get / status_get_sync / cur_pos_get
 *
 *   ── 模式切换 ────────────────────────────────────────
 *     manual_mode
 *
 *   ── 回零 ────────────────────────────────────────────
 *     home / home_set
 *
 *   ── 软件限位 ────────────────────────────────────────
 *     positive_limit / negative_limit
 *
 *   ── 故障清除 ────────────────────────────────────────
 *     fault_clear
 *
 *   ── 跟随误差监控 ────────────────────────────────────
 *     following_err_enable / following_err_limit_set
 *
 *   ── 编码器标定 ──────────────────────────────────────
 *     encoder_ratio_set
 *
 *   ── I²t 过载保护（短时大流 + 长时小流双阈） ─────────
 *     i2t_fault_enable
 *     i2t_peak_current_set / i2t_continuous_current_set
 *     i2t_peak_time_set
 */
typedef struct
{
    void (*motor_stop)(struct h_motor_base *me);
    void (*motor_start)(struct h_motor_base *me);
    void (*move_velocity_mode)(struct h_motor_base *me,
                               h_motor_base_move_dir_t dir);
    void (*move_relative_mode)(struct h_motor_base *me, double move_mm);
    void (*move_absolute_mode)(struct h_motor_base *me, double pos_mm);
    void (*velocity_set)(struct h_motor_base *me, int32_t velocity);
    void (*acce_dece_set)(struct h_motor_base *me,
                          uint32_t acce, uint32_t dece);
    void (*manual_mode)(struct h_motor_base *me, bool is_manual);

    /* 异步状态查询：触发一次 = "把当前状态打到回调里"。
     * 调用立刻返回，结果通过 status_cb 回吐。 */
    void (*status_get)(struct h_motor_base *me);

    /* 同步状态查询：阻塞读，立刻把结果写入 status 出参。
     * 用于状态机里"必须立刻拿到"的场景。 */
    bool (*status_get_sync)(struct h_motor_base *me,
                            h_motor_base_status_t *status);

    void (*cur_pos_get)(struct h_motor_base *me);
    void (*enable)(struct h_motor_base *me, bool enable);
    void (*home)(struct h_motor_base *me);
    void (*home_set)(struct h_motor_base *me, double offset_mm);
    void (*positive_limit)(struct h_motor_base *me, uint32_t limit_mm);
    void (*negative_limit)(struct h_motor_base *me, int32_t limit_mm);
    void (*fault_clear)(struct h_motor_base *me);
    void (*following_err_enable)(struct h_motor_base *me, bool enable);
    void (*following_err_limit_set)(struct h_motor_base *me,
                                    double limit_mm);
    void (*encoder_ratio_set)(struct h_motor_base *me,
                              double cnt_per_mm);
    void (*i2t_fault_enable)(struct h_motor_base *me, bool enable);

    /* I²t 三参数：peak 是短时允许的峰值电流（单位 0.01A），
     * continuous 是长时允许的稳态电流，peak_time 是允许 peak
     * 持续多久。三参数共同决定"电机被烧坏前能允许的电流-时间曲线"。 */
    void (*i2t_peak_current_set)(struct h_motor_base *me,
                                 uint16_t current_001a);
    void (*i2t_continuous_current_set)(struct h_motor_base *me,
                                       uint16_t current_001a);
    void (*i2t_peak_time_set)(struct h_motor_base *me,
                              uint16_t time_ms);
} h_motor_base_ops_t;

/* 错误码枚举。负值 + 不同绝对值方便 err_cb 内部 switch。 */
typedef enum
{
    H_MOTOR_COMMUNICATION_OK  = 0,
    H_MOTOR_COMMUNICATION_ERR = -1,
    H_MOTOR_HW_ERR            = -2,
} h_motor_base_err_id_t;

/* ───────── 三种回调签名 ─────────
 * 设计原则：每种事件类型一个独立回调。事件类型严格匹配硬件
 * 能产生的事件种类——状态变化 / 位置更新 / 错误，刚好够用。 */
typedef void (*status_cb_t)(h_motor_base_status_t *status,
                            struct h_motor_base *me);

typedef void (*cur_pos_cb_t)(double pos_mm,
                             struct h_motor_base *me);

typedef void (*err_cb_t)(h_motor_base_err_id_t err_id,
                         struct h_motor_base *me);

/* 基类对象。子类把 h_motor_base_t 放在第一字段（参考
 * h_motor_acm_t.base），向上转型零代价。
 * ops 字段加 const，与 v_motor_base_t 对齐——子类的静态 ops 表
 * 落到 .rodata 段全程序共享，构造期挂上之后基类层不再改写。 */
typedef struct h_motor_base
{
    const h_motor_base_ops_t *ops;
    status_cb_t  status_cb;
    cur_pos_cb_t cur_pos_cb;
    err_cb_t     err_cb;
} h_motor_base_t;


/* ────────── 应用层封装函数 ──────────
 * 应用层只调下面这一组 h_motor_base_xxx，永远看不到 ops 字段。
 * 每个封装函数体内部走 me->ops->xxx(me, ...) dispatch。 */

void h_motor_base_init(struct h_motor_base *me);
void h_motor_base_stop(struct h_motor_base *me);
void h_motor_base_start(struct h_motor_base *me);
void h_motor_base_move_velocity_mode(struct h_motor_base *me,
                                     h_motor_base_move_dir_t dir);
void h_motor_base_move_relative_mode(struct h_motor_base *me,
                                     double move_mm);
void h_motor_base_move_absolute_mode(struct h_motor_base *me,
                                     double pos_mm);
void h_motor_base_velocity_set(struct h_motor_base *me,
                               int32_t velocity);
void h_motor_base_acce_dece_set(struct h_motor_base *me,
                                uint32_t acce, uint32_t dece);
void h_motor_base_manual_mode(struct h_motor_base *me, bool is_manual);
void h_motor_base_status_get(struct h_motor_base *me);
bool h_motor_base_status_get_sync(struct h_motor_base *me,
                                  h_motor_base_status_t *status);
void h_motor_base_status_get_cb_register(struct h_motor_base *me,
                                         status_cb_t cb);
void h_motor_base_cur_pos_get(struct h_motor_base *me);
void h_motor_base_cur_pos_cb_register(struct h_motor_base *me,
                                      cur_pos_cb_t cb);
void h_motor_base_enable(struct h_motor_base *me, bool enable);
void h_motor_base_fault_cb_register(struct h_motor_base *me,
                                    err_cb_t cb);
void h_motor_base_home(struct h_motor_base *me);
void h_motor_base_home_set(struct h_motor_base *me,
                           double offset_mm);
void h_motor_base_positive_limit(struct h_motor_base *me,
                                 uint32_t limit_mm);
void h_motor_base_negative_limit(struct h_motor_base *me,
                                 int32_t limit_mm);
void h_motor_base_fault_clear(struct h_motor_base *me);
void h_motor_base_cur_pos_get_cb_register(struct h_motor_base *me,
                                          cur_pos_cb_t cb);
void h_motor_base_following_error_enbale(struct h_motor_base *me,
                                         bool enable);
void h_motor_base_following_err_limit_set(struct h_motor_base *me,
                                          double limit_mm);
void h_motor_base_encoder_ratio_set(struct h_motor_base *me,
                                    double cnt_per_mm);
void h_motor_base_i2t_fault_enable(struct h_motor_base *me,
                                   bool enable);
void h_motor_base_i2t_peak_current_set(struct h_motor_base *me,
                                       uint16_t current_001a);
void h_motor_base_i2t_continuous_current_set(struct h_motor_base *me,
                                             uint16_t current_001a);
void h_motor_base_i2t_peak_time_set(struct h_motor_base *me,
                                    uint16_t time_ms);

#endif /* __H_MOTOR_H */
