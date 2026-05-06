/**
  ******************************************************************************
  * @file    v_motor_ifx007t.c
  * @brief   IFX007T 双半桥驱动垂直直流电机的 PWM 调速实现。
  *
  *   ┌────────────────── 教学说明 ──────────────────┐
  *   │ 这一份是真实工程里跑了一年多的产品级版本。           │
  *   │ 跟 ch20 正文里"几行 GPIO 操作"的 OOP 教学骨架相比， │
  *   │ 多了 5 件事：                                       │
  *   │   1. PWM 调速代替直接 GPIO 推挽（H 桥两路 PWM）      │
  *   │   2. 软启动（5% -> 起步占空，避免冲击电流）         │
  *   │   3. 满速线性加速（起步占空 -> 100%）               │
  *   │   4. 减速区限位 + 三层超时保护                      │
  *   │   5. IS 引脚电流采样 + EMA 滤波过流故障判断         │
  *   │ OOP 骨架完全不变：v_motor_base_t 的 3 个 ops          │
  *   │ + 1 个错误回调，子类内部把所有这些异步行为塞进         │
  *   │ 工作线程 + 周期定时器。上层应用层永远只见             │
  *   │ v_motor_base_move(motor, UP) 这一句普通函数调用。     │
  *   └──────────────────────────────────────────────┘
  *
  *   IFX007T 双半桥真值表（ctrl_1 / ctrl_2 是两个 IN 引脚的输入信号）：
  *
  *     INH  IN   高边臂   低边臂   输出      电机端口状态
  *     ───  ──   ──────   ──────   ───────   ──────────────
  *      0   ×     OFF      OFF     高阻       全断（休眠）
  *      1   1     ON       OFF     VBAT       拉到电源
  *      1   0     OFF      ON      GND        拉到地
  *      1  PWM    ↕         ↕     脉冲        慢衰减斩波调压
  *
  *   两个 IFX007T 串成 H 桥驱动一个有刷直流电机：
  *
  *      ctrl_1 = PWM, ctrl_2 = 0  →  电机正转(UP)，速度由占空决定
  *      ctrl_1 = 0,   ctrl_2 = PWM →  电机反转(DOWN)
  *      ctrl_1 = 0,   ctrl_2 = 0   →  慢刹车（两路低边臂同时短路绕组）
  *      INH    = 0                 →  休眠（关全部 MOS，最低功耗）
  *
  *   切换方向之前必须先停一段时间（DIR_CHANGE_DELAY_MS），让电机绕组
  *   电流泄放完，否则瞬间反向会有几十安培的尖峰把 IFX007T 的过流锁死。
  ******************************************************************************
  */

#include "v_motor_ifx007t.h"
#include "platform_pin.h"
#include "platform_assert.h"

#define LOG_TAG "v_motor_pwm"
#include "elog.h"

/* ADC 通道接口由调用方提供。这里只 extern 声明本驱动需要的两个函数，
 * 让 v_motor_ifx007t.c 不强依赖某个具体 ADC 抽象层。 */
extern void  adc_channel_start(adc_channel_base_t *ch);
extern float adc_channel_cur_volt_mv_get(adc_channel_base_t *ch);


/* ================================================================== *
 *  配置宏：所有调参集中在这里。改电机型号 / 整机机械结构时，          *
 *  通常只改这一段，下面的算法不动。                                  *
 * ================================================================== */

/* PWM 周期（计数值）。和 board 层 PWM 时基组合决定 PWM 频率。
 * 真实工程跑 20kHz，刚好在人耳听阈上沿，又能让 IFX007T 的
 * 死区延时占比可忽略。100000 是 timer 计数器的 ARR + 1。 */
#define V_MOTOR_PWM_PERIOD                  (100000)

/* 软启动起点占空 5%。把电机从完全停止平滑带起，避免上电瞬间
 * 整机抖一下。 */
#define V_MOTOR_SOFT_START_DUTY             (0.05f)

/* 软启动持续时间（ms）：从 5% 平滑爬到起步占空。
 * 太短会冲击电流，太长会让用户觉得"按下按钮没反应"。
 * 500ms 是经验值。 */
#define V_MOTOR_SOFT_START_TIME_MS          (500)

/* 起步占空：上行 44%，下行 22%。
 *
 * 为什么上下不一样？因为垂直方向受重力影响：
 *   - 上行要克服自身重量 + 摩擦，扭矩需求大，44% 起步保证能动
 *   - 下行重力帮忙，22% 就够了，太大反而会把电机推得溜车
 *
 * 这两个值是真实整机调出来的，机械结构变了要重新标定。 */
#define V_MOTOR_START_DUTY_UP               (0.44f)
#define V_MOTOR_START_DUTY_DOWN             (0.22f)

/* 起步占空爬到 100% 占空所用时间（ms）。500ms 是慢爬区结束后
 * 加速到全速的时间。 */
#define V_MOTOR_RAMP_TIME_MS                (500)

/* 周期工作定时器节拍（ms）。所有加减速、故障检测、限位轮询
 * 都跑在这一拍里。10ms 是经验值——再快 CPU 占用太高、再慢
 * 加速曲线会卡顿肉眼可见。 */
#define V_MOTOR_WORK_TIMER_PERIOD           (10)

/* 减速区限位开关命中时的电平。具体板子按上拉/下拉决定。
 * 为 1 时表示"按住开关时引脚为高"。 */
#define V_MOTOR_DECE_PRESSED_LEVEL          (1)

/* 命令队列深度。50 条已经远远够用，应用层状态机一秒不会发超过
 * 几十条命令。 */
#define WORK_QUEUE_ELEM_NUM                 (50)

/* 切换方向前的等待时间。让绕组电流泄放完再反向，
 * 否则反电动势 + 反向电流会让 IFX007T 触发过流自锁。 */
#define V_MOTOR_DIR_CHANGE_DELAY_MS         (50)

/* IS 过流故障阈值（mV）。IS 引脚电压正比于母线电流（IFX007T 内部
 * 自带电流检测放大器）。1000mV 对应整机最大设计电流。 */
#define V_MOTOR_IS_FAULT_THRESHOLD_MV       (1000.0f)

/* IS 采样的 EMA 平滑系数。值越小越平滑（抗 PWM 斩波纹波），
 * 但故障响应越慢。0.1 = 上一帧权重 0.9。 */
#define V_MOTOR_IS_AVG_ALPHA                (0.1f)

/* 连续多少拍超阈才确认是真故障。10 拍 * 10ms = 100ms。
 * 单次毛刺 / 启动瞬间冲击不会被误判成故障。 */
#define V_MOTOR_IS_FAULT_CONFIRM_COUNT      (10)

/* IS 调试打印开关（=1 打开会按周期打印 ADC 原始值，
 * 标定阈值时方便观察）。生产固件保持 0。 */
#define V_MOTOR_IS_DEBUG_PRINT_EN           (0)

/* 调试打印的分频。50 * 10ms = 500ms 一行，刷屏不至于太频繁。 */
#define V_MOTOR_IS_PRINT_INTERVAL           (50)

/* IFX007T 的 INH 引脚清过温锁的最小宽度。
 * 数据手册要求 >= 4us，这里给 10ms 余量足够。 */
#define V_MOTOR_INH_RESET_DELAY_MS          (10)

/* 减速区长按超时阈值。500 拍 * 10ms = 5s。
 * 超过 5s 还压着限位开关，说明机械卡死或限位短路，
 * 强制停车并触发硬件故障回调。 */
#define V_MOTOR_DECE_TIMEOUT_TICKS          (500)

/* 单次运动总时长上限。3000 拍 * 10ms = 30s。
 * 整机最长行程不应超过 30s，超过即认为出问题。 */
#define V_MOTOR_MOTION_TIMEOUT_TICKS        (3000)


/* ================================================================== *
 *  内部消息：上层 ops 接口和 work_thread 之间的通信单元。            *
 * ================================================================== */

typedef enum
{
    V_MOTOR_PWM_MOVE = 0,        /* MOVE(dir): 启动一次方向运动 */
    V_MOTOR_PWM_STOP,            /* STOP:    立即停车           */
    V_MOTOR_PERIOD_TIMER,        /* TIMER:   周期 tick (10ms)   */
    V_MOTOR_PWM_FAULT_CLEAR,     /* CLEAR:   清除硬件故障锁     */
} v_motor_pwm_work_type_t;

typedef struct
{
    v_motor_pwm_work_type_t type;
    v_motor_base_dir_t      dir;
} v_motor_pwm_msg_t;


/* ================================================================== *
 *  静态前向声明                                                       *
 * ================================================================== */

static void _hw_pwm_all_off(v_motor_ifx007t_t *me);
static void _hw_sleep_off  (v_motor_ifx007t_t *me);
static void _hw_sleep_on   (v_motor_ifx007t_t *me);
static void _motor_stop    (v_motor_ifx007t_t *me);
static void _motor_move    (v_motor_ifx007t_t *me, v_motor_base_dir_t dir);
static void _motor_timer_timeout(v_motor_ifx007t_t *me);
static void _motor_fault_clear  (v_motor_ifx007t_t *me);
static void _calc_ramp_params   (v_motor_ifx007t_t *me, v_motor_base_dir_t dir);
static void _hw_start_pwm_output(v_motor_ifx007t_t *me, v_motor_base_dir_t dir);
static void _is_fault_check       (v_motor_ifx007t_t *me);
static void _dece_timeout_check   (v_motor_ifx007t_t *me);
static void _motion_timeout_check (v_motor_ifx007t_t *me);


/* ================================================================== *
 *  硬件操作小工具                                                     *
 * ================================================================== */

/* 关闭两路 PWM 输出：先把 pulse 写 0（占空清零，输出立即变低），
 * 再 disable 整个通道（关 timer 输出比较）。两步都做的原因是
 * 部分 MCU 的 PWM 关闭后通道电平残留，得手动把 pulse 清零。 */
static void _hw_pwm_all_off(v_motor_ifx007t_t *me)
{
    platform_pwm_set_pulse(me->ctrl_1_pwm_dev,
                           me->ctrl_1_pwm_channel, 0);
    platform_pwm_set_pulse(me->ctrl_2_pwm_dev,
                           me->ctrl_2_pwm_channel, 0);
    platform_pwm_disable(me->ctrl_1_pwm_dev,
                         me->ctrl_1_pwm_channel);
    platform_pwm_disable(me->ctrl_2_pwm_dev,
                         me->ctrl_2_pwm_channel);
}

/* INH 拉低 = 两片 IFX007T 进入休眠，整个 H 桥高阻断电。
 * 用来真正"停"电机或者清过温锁。 */
static void _hw_sleep_off(v_motor_ifx007t_t *me)
{
    platform_pin_write(me->sleep1_pin, 0);
    platform_pin_write(me->sleep2_pin, 0);
}

/* INH 拉高 = 唤醒 IFX007T，进入工作状态。运动前必须先唤醒，
 * 否则 PWM 信号到 IN 引脚也没用。 */
static void _hw_sleep_on(v_motor_ifx007t_t *me)
{
    platform_pin_write(me->sleep1_pin, 1);
    platform_pin_write(me->sleep2_pin, 1);
}


/* ================================================================== *
 *  IS 引脚电流采样 + EMA 滤波 + 连续超阈确认                         *
 * ================================================================== */
/* 调用方：每 10ms 由 _motor_timer_timeout() 调一次。                  */

static void _is_fault_check(v_motor_ifx007t_t *me)
{
    /* 没接 IS 通道（构造时传 NULL），跳过过流保护。
     * 教学样本里大多没接 ADC，这里允许优雅降级。 */
    if (NULL == me->is_adc_channel)
    {
        return;
    }

    /* 已经触发过故障锁，不再重复处理。
     * 必须等 fault_clear 显式解除。 */
    if (me->is_hw_error)
    {
        return;
    }

    float raw_mv = adc_channel_cur_volt_mv_get(me->is_adc_channel);

    /* EMA（指数移动平均）滤波：avg = α·raw + (1-α)·avg
     * α 小 = 平滑（抗 PWM 斩波纹波），α 大 = 响应快。
     * 这里 α=0.1，等价于一阶低通约 16Hz 截止，远低于 PWM 20kHz。 */
    me->is_avg_mv = V_MOTOR_IS_AVG_ALPHA * raw_mv
        + (1.0f - V_MOTOR_IS_AVG_ALPHA) * me->is_avg_mv;

#if V_MOTOR_IS_DEBUG_PRINT_EN
    me->is_print_count++;
    if (me->is_print_count >= V_MOTOR_IS_PRINT_INTERVAL)
    {
        me->is_print_count = 0;
        log_d("IS ADC: raw=%.1fmV avg=%.1fmV dir=%d",
              raw_mv, me->is_avg_mv, me->work_dir);
    }
#endif

    /* 连续 10 拍（100ms）都超阈值才确认是真故障。
     * 单次毛刺 / 启动冲击电流不会被误判。 */
    if (me->is_avg_mv > V_MOTOR_IS_FAULT_THRESHOLD_MV)
    {
        me->is_fault_count++;
        if (me->is_fault_count >= V_MOTOR_IS_FAULT_CONFIRM_COUNT)
        {
            log_e("IS fault: avg=%.1fmV thr=%.1fmV dir=%d",
                  me->is_avg_mv,
                  V_MOTOR_IS_FAULT_THRESHOLD_MV,
                  me->work_dir);

            _motor_stop(me);
            me->is_hw_error = true;

            /* 通过基类 err_cb 把故障吐到上层。 */
            if (me->base.err_cb)
            {
                me->base.err_cb(V_MOTOR_HW_ERR, &me->base);
            }
        }
    }
    else
    {
        /* 一旦掉回阈值以下，重新计数，避免被偶发尖峰累积触发。 */
        me->is_fault_count = 0;
    }
}


/* ================================================================== *
 *  减速区限位开关长按超时（5s）                                      *
 * ================================================================== */
/* 正常情况：电机进减速区后 PWM 缓慢退到起步占空，几百毫秒内
 *           应该跑完限位开关之后到达终点开关，限位开关变高 -> 低。
 * 异常情况：限位一直被压着 5s 不松开，机械卡死或限位短路。
 *           强制停车并报硬件故障。 */

static void _dece_timeout_check(v_motor_ifx007t_t *me)
{
    bool top_pressed =
        (platform_pin_read(me->top_dece_pin)
            == V_MOTOR_DECE_PRESSED_LEVEL);
    bool bottom_pressed =
        (platform_pin_read(me->bottom_dece_pin)
            == V_MOTOR_DECE_PRESSED_LEVEL);

    if (top_pressed || bottom_pressed)
    {
        me->dece_timeout_count++;
        if (me->dece_timeout_count >= V_MOTOR_DECE_TIMEOUT_TICKS)
        {
            log_e("Dece zone timeout: top=%d bot=%d dir=%d cnt=%lu",
                  top_pressed, bottom_pressed,
                  me->work_dir,
                  me->dece_timeout_count);
            _motor_stop(me);
            me->is_hw_error = true;
            if (me->base.err_cb)
            {
                me->base.err_cb(V_MOTOR_HW_ERR, &me->base);
            }
        }
    }
    else
    {
        /* 限位松开就清零。不要累积，否则两次正常进减速区
         * 加起来超 5s 也会被误判。 */
        me->dece_timeout_count = 0;
    }
}


/* ================================================================== *
 *  整体运动总时长超时（30s）                                          *
 * ================================================================== */
/* 任何一次 motor_move 启动后 30s 还没到位（没 motor_stop / 没踩限位
 * 后停下），认为出大问题，强制停车 + 报错。 */

static void _motion_timeout_check(v_motor_ifx007t_t *me)
{
    me->motion_timeout_count++;
    if (me->motion_timeout_count >= V_MOTOR_MOTION_TIMEOUT_TICKS)
    {
        log_e("Motion timeout: dir=%d cnt=%lu",
              me->work_dir, me->motion_timeout_count);
        _motor_stop(me);
        me->is_hw_error = true;
        if (me->base.err_cb)
        {
            me->base.err_cb(V_MOTOR_HW_ERR, &me->base);
        }
    }
}


/* ================================================================== *
 *  核心电机动作                                                       *
 * ================================================================== */

/* 立即停车。
 *   1. 停 work_timer，防止下一拍 TIMER 消息再进队列；
 *   2. 清空命令队列，让队列里堆积的旧命令不再生效；
 *   3. PWM 输出全关；
 *   4. osDelay(10) 等死区时间，让绕组电流泄放干净；
 *   5. INH 拉低进休眠；
 *   6. 复位所有运动期间的状态计数器。 */
static void _motor_stop(v_motor_ifx007t_t *me)
{
    osTimerStop(me->work_timer);
    osMessageQueueReset(me->work_queue);

    _hw_pwm_all_off(me);
    osDelay(10);

    _hw_sleep_off(me);

    me->work_dir = V_MOTOR_BASE_MOVE_STOP;
    me->dece_timeout_count = 0;
    me->motion_timeout_count = 0;
}

/* 计算当前方向下两段加速曲线的所有 step。
 *
 *   Phase 1 (软启动): pwm_pulse 从 5% 占空线性爬到起步占空
 *                     用 V_MOTOR_SOFT_START_TIME_MS 跑完
 *                     每拍递增 soft_start_step_size
 *
 *   Phase 2 (满速加速): pwm_pulse 从起步占空线性爬到 100%
 *                       用 V_MOTOR_RAMP_TIME_MS 跑完
 *                       每拍递增 current_step_size
 *
 *   起步 pulse 由方向决定：上行 44%，下行 22%（重力补偿）。 */
static void _calc_ramp_params(v_motor_ifx007t_t *me,
                              v_motor_base_dir_t dir)
{
    uint32_t soft_total_steps =
        V_MOTOR_SOFT_START_TIME_MS / V_MOTOR_WORK_TIMER_PERIOD;
    if (soft_total_steps == 0) soft_total_steps = 1;

    uint32_t ramp_total_steps =
        V_MOTOR_RAMP_TIME_MS / V_MOTOR_WORK_TIMER_PERIOD;
    if (ramp_total_steps == 0) ramp_total_steps = 1;

    me->soft_start_min_pulse =
        (uint32_t)(V_MOTOR_PWM_PERIOD * V_MOTOR_SOFT_START_DUTY);

    if (V_MOTOR_BASE_MOVE_UP == dir)
    {
        me->current_min_pulse =
            (uint32_t)(V_MOTOR_PWM_PERIOD * V_MOTOR_START_DUTY_UP);
    }
    else
    {
        me->current_min_pulse =
            (uint32_t)(V_MOTOR_PWM_PERIOD * V_MOTOR_START_DUTY_DOWN);
    }

    /* 防御：起步占空 < 软启动起点（极端配置），软启动一拍跳完。 */
    if (me->current_min_pulse > me->soft_start_min_pulse)
    {
        me->soft_start_step_size =
            (me->current_min_pulse - me->soft_start_min_pulse)
            / soft_total_steps;
    }
    else
    {
        me->soft_start_step_size = 0;
    }

    if (V_MOTOR_PWM_PERIOD > me->current_min_pulse)
    {
        me->current_step_size =
            (V_MOTOR_PWM_PERIOD - me->current_min_pulse)
            / ramp_total_steps;
    }
    else
    {
        me->current_step_size = 0;
    }

    /* 起跑点：5% 占空。从这里开始软启动。 */
    me->pwm_pulse = me->soft_start_min_pulse;
}

/* 启动选定方向的 PWM 输出。
 *   UP   :  ctrl_1 = pwm_pulse, ctrl_2 = 0  (慢衰减斩波正向)
 *   DOWN :  ctrl_1 = 0,         ctrl_2 = pwm_pulse  (反向) */
static void _hw_start_pwm_output(v_motor_ifx007t_t *me,
                                 v_motor_base_dir_t dir)
{
    if (V_MOTOR_BASE_MOVE_UP == dir)
    {
        platform_pwm_set_pulse(me->ctrl_2_pwm_dev,
                               me->ctrl_2_pwm_channel, 0);
        platform_pwm_disable(me->ctrl_2_pwm_dev,
                             me->ctrl_2_pwm_channel);

        platform_pwm_set_pulse(me->ctrl_1_pwm_dev,
                               me->ctrl_1_pwm_channel,
                               me->pwm_pulse);
        platform_pwm_enable(me->ctrl_1_pwm_dev,
                            me->ctrl_1_pwm_channel);
    }
    else
    {
        platform_pwm_set_pulse(me->ctrl_1_pwm_dev,
                               me->ctrl_1_pwm_channel, 0);
        platform_pwm_disable(me->ctrl_1_pwm_dev,
                             me->ctrl_1_pwm_channel);

        platform_pwm_set_pulse(me->ctrl_2_pwm_dev,
                               me->ctrl_2_pwm_channel,
                               me->pwm_pulse);
        platform_pwm_enable(me->ctrl_2_pwm_dev,
                            me->ctrl_2_pwm_channel);
    }
}

/* 启动一次方向运动（在 work_thread 上下文里运行）。 */
static void _motor_move(v_motor_ifx007t_t *me,
                        v_motor_base_dir_t dir)
{
    if (me->is_hw_error)
    {
        log_e("Motor move rejected: HW error active");
        return;
    }

    /* 同方向且已经在跑，幂等返回。 */
    if (dir == me->work_dir && osTimerIsRunning(me->work_timer))
    {
        return;
    }

    /*
     * 注意：先停 timer 防止下一拍 TIMER 消息进队列，但故意不
     * Reset 队列。原因——上层调用方那边可能已经把
     * STOP 消息排在队列里，绝不能丢。残留的 TIMER 消息无害，
     * 因为 work_dir 和 ramp 参数已经按新方向算过，下一拍
     * _motor_timer_timeout() 跑出来最多多走一个加速 step。
     */
    osTimerStop(me->work_timer);

    /* 切换方向必须先关 PWM + 等死区，让绕组电流泄放完。
     * 否则反电动势 + 反向电流瞬间会让 IFX007T 过流自锁。 */
    if (me->work_dir != V_MOTOR_BASE_MOVE_STOP
        && me->work_dir != dir)
    {
        _hw_pwm_all_off(me);
        osDelay(V_MOTOR_DIR_CHANGE_DELAY_MS);
    }

    me->work_dir = dir;

    _hw_sleep_on(me);
    osDelay(2);                /* INH 拉高到 PWM 起跑的最小延时 */

    _calc_ramp_params(me, dir);
    _hw_start_pwm_output(me, dir);

    /* 新一段运动开始，复位所有保护状态。 */
    me->is_avg_mv = 0.0f;
    me->is_fault_count = 0;
    me->is_print_count = 0;
    me->dece_timeout_count = 0;
    me->motion_timeout_count = 0;

    osTimerStart(me->work_timer, V_MOTOR_WORK_TIMER_PERIOD);
}

/* 周期 tick 处理（10ms 一次）。
 *
 * 顺序：
 *   1. 三层故障检测，任一层触发就立刻退出（内部已停车）；
 *   2. 看当前活动方向对应的减速区开关：
 *        踩在减速区  ->  PWM 退到起步占空（慢爬）
 *        离开减速区  ->  PWM 升到 100%（满速）
 *      两个方向都要兼容"反向"情形（电机被人手推回去）：
 *      pulse 比 current_min_pulse 小，沿软启动 step_size 升到 min。
 */
static void _motor_timer_timeout(v_motor_ifx007t_t *me)
{
    platform_device_pwm_t active_pwm_device;
    int32_t active_pwm_channel;
    int32_t dece_pin;

    if (V_MOTOR_BASE_MOVE_STOP == me->work_dir)
    {
        return;
    }

    /* 三层保护按严重程度从轻到重检查。任何一层触发都已经
     * 在内部 _motor_stop()，这里只需提前返回。 */
    _is_fault_check(me);
    if (me->is_hw_error) { return; }

    _dece_timeout_check(me);
    if (me->is_hw_error) { return; }

    _motion_timeout_check(me);
    if (me->is_hw_error) { return; }

    /* 选当前激活方向对应的 PWM + 限位引脚。 */
    if (V_MOTOR_BASE_MOVE_UP == me->work_dir)
    {
        active_pwm_device  = me->ctrl_1_pwm_dev;
        active_pwm_channel = me->ctrl_1_pwm_channel;
        dece_pin           = me->top_dece_pin;
    }
    else
    {
        active_pwm_device  = me->ctrl_2_pwm_dev;
        active_pwm_channel = me->ctrl_2_pwm_channel;
        dece_pin           = me->bottom_dece_pin;
    }

    if (platform_pin_read(dece_pin) == V_MOTOR_DECE_PRESSED_LEVEL)
    {
        /* 踩在减速区：把 pulse 退到 current_min_pulse（起步占空），
         * 慢爬到位。退的过程也是按 current_step_size 一拍一拍降。 */
        if (me->pwm_pulse > me->current_min_pulse)
        {
            if (me->pwm_pulse
                > (me->current_min_pulse + me->current_step_size))
            {
                me->pwm_pulse -= me->current_step_size;
            }
            else
            {
                me->pwm_pulse = me->current_min_pulse;
            }
        }
        /* 边界情况：电机当前 pulse 比起步占空还低（比如刚启动还
         * 在软启动阶段就进了减速区），按软启动 step_size 升到 min。 */
        else if (me->pwm_pulse < me->current_min_pulse)
        {
            if ((me->pwm_pulse + me->soft_start_step_size)
                < me->current_min_pulse)
            {
                me->pwm_pulse += me->soft_start_step_size;
            }
            else
            {
                me->pwm_pulse = me->current_min_pulse;
            }
        }
    }
    else
    {
        /* 不在减速区：先把软启动跑完，再线性加速到 100%。 */
        if (me->pwm_pulse < me->current_min_pulse)
        {
            /* 阶段一：软启动 */
            if ((me->pwm_pulse + me->soft_start_step_size)
                < me->current_min_pulse)
            {
                me->pwm_pulse += me->soft_start_step_size;
            }
            else
            {
                me->pwm_pulse = me->current_min_pulse;
            }
        }
        else if (me->pwm_pulse < V_MOTOR_PWM_PERIOD)
        {
            /* 阶段二：满速加速 */
            if ((me->pwm_pulse + me->current_step_size)
                < V_MOTOR_PWM_PERIOD)
            {
                me->pwm_pulse += me->current_step_size;
            }
            else
            {
                me->pwm_pulse = V_MOTOR_PWM_PERIOD;
            }
        }
        /* 已到 100%，保持。 */
    }

    /* 把这一拍算出来的新 pulse 写到激活通道。
     * period 不变（已在 init 里设好），只更新 pulse 极轻量。 */
    platform_pwm_set_pulse(active_pwm_device,
                           active_pwm_channel,
                           me->pwm_pulse);
}

/* 清除硬件故障锁。
 *
 *   IFX007T 的过温保护是"锁住"型——一旦触发，必须把 INH 拉低
 *   再拉高才能解锁。数据手册要求 INH 低电平至少 4us，
 *   这里给 10ms 大余量，肯定够。
 *
 *   除了硬件锁，软件层也要把 is_hw_error / 各计数器清零，
 *   否则下次 motor_move 还是会被 is_hw_error 拒绝。 */
static void _motor_fault_clear(v_motor_ifx007t_t *me)
{
    log_i("Fault clear: INH reset start");

    _motor_stop(me);

    _hw_sleep_off(me);
    osDelay(V_MOTOR_INH_RESET_DELAY_MS);

    me->is_hw_error          = false;
    me->is_avg_mv            = 0.0f;
    me->is_fault_count       = 0;
    me->is_print_count       = 0;
    me->dece_timeout_count   = 0;
    me->motion_timeout_count = 0;

    log_i("Fault clear: INH reset done");
}


/* ================================================================== *
 *  ops 接口（v_motor_base 的 3 个虚方法）。                          *
 *  上层调用方（应用层 / 状态机）调到这里。                            *
 *  这一层只做一件事：把命令丢进队列就立刻返回，                       *
 *  让上层调用是非阻塞的。真正的硬件操作在 work_thread 里串行执行。     *
 * ================================================================== */

static void motor_stop(v_motor_base_t *base)
{
    v_motor_ifx007t_t *me = (v_motor_ifx007t_t *)base;
    v_motor_pwm_msg_t msg;
    msg.type = V_MOTOR_PWM_STOP;
    msg.dir  = V_MOTOR_BASE_MOVE_STOP;
    osMessageQueuePut(me->work_queue, &msg, 0, 0);
}

static void motor_move(v_motor_base_t *base, v_motor_base_dir_t dir)
{
    v_motor_ifx007t_t *me = (v_motor_ifx007t_t *)base;
    v_motor_pwm_msg_t msg;
    msg.type = V_MOTOR_PWM_MOVE;
    msg.dir  = dir;
    osMessageQueuePut(me->work_queue, &msg, 0, 0);
}

static void motor_fault_clear(v_motor_base_t *base)
{
    v_motor_ifx007t_t *me = (v_motor_ifx007t_t *)base;
    v_motor_pwm_msg_t msg;
    msg.type = V_MOTOR_PWM_FAULT_CLEAR;
    msg.dir  = V_MOTOR_BASE_MOVE_STOP;
    osMessageQueuePut(me->work_queue, &msg, 0, 0);
}

/* 静态 ops 表 —— 整个进程只一份，所有 v_motor_ifx007t_t 实例共享。 */
static const v_motor_base_ops_t ops =
{
    .motor_stop  = motor_stop,
    .motor_move  = motor_move,
    .fault_clear = motor_fault_clear,
};


/* ================================================================== *
 *  工作线程 + 周期定时器                                              *
 * ================================================================== */

/* 工作线程主循环。所有命令在这里串行执行，
 * 天然避免了多线程并发访问 PWM/GPIO 的同步问题。 */
static void work_thread(void *argument)
{
    v_motor_ifx007t_t *me = (v_motor_ifx007t_t *)argument;
    v_motor_pwm_msg_t msg;

    for (;;)
    {
        osMessageQueueGet(me->work_queue, &msg, NULL, osWaitForever);
        switch (msg.type)
        {
        case V_MOTOR_PWM_MOVE:
            _motor_move(me, msg.dir);
            break;

        case V_MOTOR_PWM_STOP:
            _motor_stop(me);
            break;

        case V_MOTOR_PERIOD_TIMER:
            _motor_timer_timeout(me);
            break;

        case V_MOTOR_PWM_FAULT_CLEAR:
            _motor_fault_clear(me);
            break;

        default:
            break;
        }
    }
}

/* 周期定时器回调（运行在 RTOS timer 服务线程上下文）。
 * 不直接做硬件操作，只是把"该跑一拍"这个事件丢进 work_queue，
 * 让 work_thread 串行处理。这是经典的"中断/定时器只产生事件，
 * 实际工作在专用线程里做"模式。 */
static void v_motor_ifx007t_work_timer(void *argument)
{
    v_motor_ifx007t_t *me = (v_motor_ifx007t_t *)argument;
    v_motor_pwm_msg_t msg;
    msg.type = V_MOTOR_PERIOD_TIMER;
    msg.dir  = V_MOTOR_BASE_MOVE_STOP;
    osMessageQueuePut(me->work_queue, &msg, 0, 0);
}


/* ================================================================== *
 *  构造函数                                                           *
 * ================================================================== */

void v_motor_ifx007t_init(v_motor_ifx007t_t *me,
                          v_motor_ifx007t_init_param_t *init_param)
{
    const osTimerAttr_t timer_attr =
    {
        .name      = "v_motor_pwm_tmr",
        .attr_bits = 0,
        .cb_mem    = NULL,
        .cb_size   = 0,
    };

    osThreadAttr_t work_thread_attr =
    {
        .name       = "v_motor_work",
        .attr_bits  = osThreadDetached,
        .priority   = osPriorityAboveNormal,
        .stack_size = 2048,
    };

    /* 先把基类清干净，再挂 ops 表。
     * 顺序很重要：base init 会把 ops 字段清成 NULL。 */
    v_motor_init(&me->base);
    me->base.ops = &ops;

    /* 两路 PWM 通道号 */
    me->ctrl_1_pwm_channel = init_param->ctrl_1_pwm_channel;
    me->ctrl_2_pwm_channel = init_param->ctrl_2_pwm_channel;

    /* 按字符串名字查 PWM 设备指针。这是 platform 抽象层
     * 的核心：上层永远用 "pwm_tim1" 这种字符串名，不直接拿外设
     * 寄存器地址；换 MCU 只改 board 层注册表。 */
    me->ctrl_1_pwm_dev = (platform_device_pwm_t)
        platform_device_find(init_param->ctrl_1_pwm_dev_name);
    platform_assert(me->ctrl_1_pwm_dev != NULL);

    me->ctrl_2_pwm_dev = (platform_device_pwm_t)
        platform_device_find(init_param->ctrl_2_pwm_dev_name);
    platform_assert(me->ctrl_2_pwm_dev != NULL);

    /* 初始化所有运行时状态。 */
    me->pwm_pulse              = 0;
    me->current_min_pulse      = 0;
    me->current_step_size      = 0;
    me->soft_start_min_pulse   = 0;
    me->soft_start_step_size   = 0;
    me->work_dir               = V_MOTOR_BASE_MOVE_STOP;

    /* PWM 上电默认状态：disable + period 设好 + pulse=0。
     * 这样后面 _hw_start_pwm_output 只需要写 pulse + enable。 */
    platform_pwm_disable(me->ctrl_1_pwm_dev,
                         me->ctrl_1_pwm_channel);
    platform_pwm_set(me->ctrl_1_pwm_dev,
                     me->ctrl_1_pwm_channel,
                     V_MOTOR_PWM_PERIOD, 0);

    platform_pwm_disable(me->ctrl_2_pwm_dev,
                         me->ctrl_2_pwm_channel);
    platform_pwm_set(me->ctrl_2_pwm_dev,
                     me->ctrl_2_pwm_channel,
                     V_MOTOR_PWM_PERIOD, 0);

    /* 减速区限位开关：输入引脚。 */
    me->top_dece_pin =
        platform_pin_get(init_param->top_dece_pin_name);
    platform_assert(me->top_dece_pin >= 0);
    platform_pin_mode(me->top_dece_pin, PIN_MODE_INPUT);

    me->bottom_dece_pin =
        platform_pin_get(init_param->bottom_dece_pin_name);
    platform_assert(me->bottom_dece_pin >= 0);
    platform_pin_mode(me->bottom_dece_pin, PIN_MODE_INPUT);

    /* INH 引脚：输出，默认低（休眠）。 */
    me->sleep1_pin =
        platform_pin_get(init_param->sleep1_pin_name);
    platform_assert(me->sleep1_pin >= 0);
    platform_pin_write(me->sleep1_pin, 0);
    platform_pin_mode(me->sleep1_pin, PIN_MODE_OUTPUT);

    me->sleep2_pin =
        platform_pin_get(init_param->sleep2_pin_name);
    platform_assert(me->sleep2_pin >= 0);
    platform_pin_write(me->sleep2_pin, 0);
    platform_pin_mode(me->sleep2_pin, PIN_MODE_OUTPUT);

    /* IS 故障检测状态。 */
    me->is_adc_channel       = init_param->is_adc_channel;
    me->is_avg_mv            = 0.0f;
    me->is_fault_count       = 0;
    me->is_print_count       = 0;
    me->is_hw_error          = false;
    me->dece_timeout_count   = 0;
    me->motion_timeout_count = 0;

    if (me->is_adc_channel != NULL)
    {
        adc_channel_start(me->is_adc_channel);
    }

    /* 周期定时器：先 New 不 Start，等 motor_move 时才启动。 */
    me->work_timer = osTimerNew(v_motor_ifx007t_work_timer,
                                osTimerPeriodic, me, &timer_attr);
    platform_assert(me->work_timer != NULL);

    /* 命令队列 + 工作线程。线程一启动就堵在 osMessageQueueGet
     * 上等命令进来。 */
    me->work_queue = osMessageQueueNew(WORK_QUEUE_ELEM_NUM,
                                       sizeof(v_motor_pwm_msg_t),
                                       NULL);
    platform_assert(me->work_queue != NULL);

    me->work_thread = osThreadNew(work_thread, me, &work_thread_attr);
    platform_assert(me->work_thread != NULL);
}

/******************** END OF FILE ******************END OF FILE****/
