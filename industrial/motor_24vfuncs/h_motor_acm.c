/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    h_motor_acm.c
  * @brief   ACM 伺服电机 24 个虚方法的具体实现 + 双线程 + ASCII 协议帧栈。
  *          全文件接近 1500 行，但可以按"大模块"读：
  *
  *   ── 模块 1：协议层 ─────────────────────────────────────
  *      packet_parser_input              字节流解帧（按 \r 切包）
  *      get_cmd_u32_rcv / s32_rcv        阻塞读响应（u32 / s32）
  *      *_packet_send                    发命令 + 重传 3 次
  *      u32_reg_set / s32_reg_set        寄存器写
  *      u32_reg_get / s32_reg_get        寄存器读
  *      t_cmd_set                        无参 t 命令
  *      err_cb_check                     通信状态回调汇总
  *
  *   ── 模块 2：接收线程 ─────────────────────────────────
  *      rx_thread                        从 UART read，喂给 packet_parser
  *
  *   ── 模块 3：每个虚方法两份实现 ────────────────────────
  *      _xxx     (内部，运行在 work_thread 上下文，真做事)
  *      xxx      (外部，挂 ops 表，把命令丢进 work_queue 立刻返回)
  *      24 个方法成对出现，逻辑相同：
  *        外层 = "塞进队列"
  *        内层 = "组帧 → 发 → 阻塞读响应 → 错误回调"
  *
  *   ── 模块 4：故障诊断表 ───────────────────────────────
  *      evt_bits[] / fault_bits[]        事件 / 故障寄存器位含义查找表
  *      _fault_diag                      读两个寄存器解码并打 log
  *
  *   ── 模块 5：ops 表 + work_thread + 构造函数 ──────────
  *      ops                              24 个外层函数指针的静态表
  *      work_thread                      取队列一条 → 调对应 _xxx
  *      h_motor_acm_init                 构造，启动两线程，探 0x132 兼容性
  *
  *   命名规则：双下划线前缀 _xxx 是"work_thread 内部跑的实做"，
  *   不带下划线的 xxx 是"挂在 ops 表上的非阻塞入口"。
  ******************************************************************************
  */

#include "h_motor_acm.h"
#include "platform_assert.h"
#include "cmsis_os2.h"
#include "platform_pin.h"
#include "elog.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* 各路使能引脚的"非工作"默认电平。上电期间 GPIO 拉到这里。 */
#define CLUTCH_ENABLE_LEVEL                             (0)
#define REV_EN_PIN_DEFAULT_LEVEL                        (0)
#define EN_PIN_DEFAULT_LEVEL                            (0)
#define FWD_PIN_DEFAULT_LEVEL                           (0)

/* 命令 / 响应队列深度。 */
#define WORK_QUEUE_ELEM_NUM                             (30)
#define RX_QUEUE_ELEM_NUM                               (1)

/* 字节解帧的相邻字节最大间隔（ms）。超出视为新帧起点。 */
#define PACKET_PASER_RCV_BYTE_TIMEOUT_CFG               (30)
#define WORK_BUF_SIZE                                   (20)

/* 一次响应等待超时（ms）。 */
#define RX_DATA_TIMEOUT_CFG                             (50)

/* 设置类命令的成功响应固定为 "ok"。 */
#define OK_STR_LEN                                      (2)

/* 命令重传次数（首发 + 2 次重发 = 3 次）。
 * 工业现场 EMI 干扰偶尔会丢字节，3 次重传基本能覆盖。 */
#define RETRANSMISSION_TIMES                            (3)

/* 故障引脚被检测到为故障时的电平。 */
#define FAULT_DETECT_PIN_FAULT_LEVEL                    (1)

/* 编码器默认比例：1mm = 189 个编码器脉冲。
 * 跟具体机械传动比相关，应用层可通过 encoder_ratio_set 改写。 */
#define DEFAULT_ENCODER_CNT_PER_MM                      (189.0)

/* 调试期开启会按周期读取电机电流 / I²t / 跟随误差，
 * 用于参数调优。生产固件保持 0。 */
#define H_MOTOR_ACM_DEBUG                               (0)

/* ── ACM 伺服厂家 寄存器地址 ──────────────────────────────
 * 这一组宏是厂家私有协议规定的寄存器映射，拷贝自数据手册。
 * 改寄存器需对照厂家最新版手册。 */
#define ACM_REG_ACTUAL_POSITION             (0x17)  /* 当前位置             */
#define ACM_REG_PEAK_CURRENT_LIMIT          (0x21)  /* I²t 峰值电流上限     */
#define ACM_REG_CONTINUOUS_CURRENT_LIMIT    (0x22)  /* I²t 持续电流上限     */
#define ACM_REG_PEAK_CURRENT_TIME           (0x23)  /* I²t 峰值允许时间     */
#define ACM_REG_FOLLOWING_ERROR             (0x35)  /* 当前跟随误差         */
#define ACM_REG_ACTUAL_MOTOR_CURRENT        (0x38)  /* 当前电机电流         */
#define ACM_REG_EVENT_STATUS                (0xA0)  /* 实时事件位           */
#define ACM_REG_LATCHED_EVENT_STATUS        (0xA1)  /* 锁存事件位           */
#define ACM_REG_LATCHING_FAULT_STATUS       (0xA4)  /* 锁存故障位           */
#define ACM_REG_FAULT_MASK                  (0xA7)  /* 故障屏蔽掩码         */
#define ACM_REG_I2T_RUNNING_SUM             (0x132) /* I²t 累积和(老固件无) */

/* 故障屏蔽位定义（ACM_REG_FAULT_MASK 的位含义）。 */
#define ACM_FAULT_MASK_I2T_BIT              (1U << 10)
#define ACM_FAULT_MASK_FOLLOWING_ERR_BIT    (1U << 9)

/* H_MOTOR_ACM_DEBUG 模式下打印分频。5 拍 = 5 次 status 查询。 */
#define ACM_DEBUG_PRINT_INTERVAL            (5)

/* work_id_t = 命令枚举。每个 ACM_xxx 对应一个虚方法。
 * 应用层调虚方法 → 外层 xxx() 把 (work_id, work_param) 打包
 * 推进 work_queue → work_thread 取出来 switch 到对应 _xxx()
 * 真做事。这一份枚举 + work_thread 的 switch 就是"24 虚方法
 * 异步翻译成阻塞接口"的具体兑现机制（见书 ch20 § 20.2）。 */
typedef enum
{
    ACM_STOP = 0,
    ACM_START,
    ACM_MOVE_VELOCITY_MODE,
    ACM_MOVE_RELATIVE_MODE,
    ACM_MOVE_ABSOLUTE_MODE,
    ACM_VELOCITY_SET,
    ACM_ACCE_DECE_SET,
    ACM_MANUAL_MODE,
    ACM_STATUS_GET,
    ACM_CUR_POS_GET,
    ACM_ENABLE,
    ACM_HOME,
    ACM_HOME_SET,
    ACM_POSITIVE_LIMIT,
    ACM_NEGATIVE_LIMIT,
    ACM_FAULT_CLEAR,
    ACM_FOLLOWING_ERROR_ENABLE,
    ACM_FOLLOWING_ERR_LIMIT_SET,
    ACM_ENCODER_RATIO_SET,
    ACM_I2T_FAULT_ENABLE,
    ACM_I2T_PEAK_CURRENT_SET,
    ACM_I2T_CONTINUOUS_CURRENT_SET,
    ACM_I2T_PEAK_TIME_SET,
    ACM_FAULT_DIAG,
}work_id_t;

/* 命令参数 union：让所有 24 个虚方法共用一个固定大小的参数槽，
 * 避免变长 struct 进出消息队列时的复杂性。 */
typedef union
{
    uint32_t u32_data[2];
    int32_t  s32_data;
    double   double_data;
} work_param_t;

/* 一条工作消息：work_id 选哪个虚方法，work_param 是参数。 */
typedef struct
{
    work_id_t work_id;
    work_param_t work_param;
} acm_work_t;

/* 一条接收响应：data 是 ASCII 帧（已去掉 \r），len 是字节数。 */
typedef struct
{
    char data[RX_DATA_SIZE_MAX];
    uint8_t len;
} rx_data_t;


/* ============================================================ *
 *  模块 1：协议层 - ASCII 帧组装、阻塞读响应、寄存器读写       *
 * ============================================================ */

/* 字节流喂帧解析器。两字节间隔 > 30ms 就丢掉之前累积的字节
 * 重新开包；遇到 \r 就把当前累积的字节当成一帧丢进 rx_queue。
 * 在 rx_thread 上下文里被调，每个收到的字节调一次。 */
static void packet_parser_input(struct h_motor_acm *me, uint8_t data)
{
    rx_data_t rx_data;
    
    if (osKernelGetTickCount() - me->rcv_last_byte_ticks
            > PACKET_PASER_RCV_BYTE_TIMEOUT_CFG)
    {
        me->rx_buf.pos = 0;
    }
    
    if (RX_DATA_SIZE_MAX == me->rx_buf.pos) goto exit;
    
    me->rx_buf.buf[me->rx_buf.pos] = data;
    me->rx_buf.pos += 1;
    me->rcv_last_byte_ticks = osKernelGetTickCount();
    
    if (data == '\r')
    {
        memcpy(rx_data.data, me->rx_buf.buf, me->rx_buf.pos);
        rx_data.data[me->rx_buf.pos - 1] = '\0';
        rx_data.len = me->rx_buf.pos - 1;
        me->rx_buf.pos = 0;
        osMessageQueuePut(me->rx_queue, &rx_data, 0U, 0);
    }    
exit:
    return;
}

static void rcv_queue_reset(struct h_motor_acm *me)
{
    osMessageQueueReset(me->rx_queue);
}

static platform_err_t set_cmd_rcv(struct h_motor_acm *me)
{
    platform_err_t ret = PLATFORM_EOK;
    rx_data_t rx_data;
    
    osStatus_t status 
        = osMessageQueueGet(me->rx_queue, &rx_data, NULL, RX_DATA_TIMEOUT_CFG);
    if (status != osOK)
    {
        ret = PLATFORM_ETIMEOUT;
        goto exit;
    }
    
    if (rx_data.len != OK_STR_LEN || strcmp("ok", rx_data.data))
    {
        ret = PLATFORM_EINVAL;
        goto exit;
    }
exit:
    return ret;
}

static platform_err_t get_cmd_u32_rcv(struct h_motor_acm *me, uint32_t *data)
{
    platform_err_t ret = PLATFORM_EOK;
    rx_data_t rx_data;
    
    osStatus_t status 
        = osMessageQueueGet(me->rx_queue, &rx_data, NULL, RX_DATA_TIMEOUT_CFG);
    if (status != osOK)
    {
        ret = PLATFORM_ETIMEOUT;
        goto exit;
    }
    if (rx_data.data[0] != 'v' || rx_data.data[1] != ' ')
    {
        ret = PLATFORM_EINVAL;
        goto exit;
    }
    char *end_pos;
    *data = strtoul(rx_data.data + 2, &end_pos, 10);
exit:
    return ret;
}

static platform_err_t get_cmd_u32_packet_send(struct h_motor_acm *me, 
                                char *str, uint32_t len, uint32_t *data)
{
    platform_err_t ret = PLATFORM_EOK;
    
    for(uint32_t i = 0; i < RETRANSMISSION_TIMES; i++)
    {
        rcv_queue_reset(me);
        platform_device_write(me->uart_dev, 0, str, len);
        ret = get_cmd_u32_rcv(me, data);
        
        if(PLATFORM_EOK == ret)  break;
    }

    return ret;
}

static platform_err_t get_cmd_s32_rcv(struct h_motor_acm *me, int32_t *data)
{
    platform_err_t ret = PLATFORM_EOK;
    rx_data_t rx_data;
    
    osStatus_t status 
        = osMessageQueueGet(me->rx_queue, &rx_data, NULL, RX_DATA_TIMEOUT_CFG);
    if (status != osOK)
    {
        ret = PLATFORM_ETIMEOUT;
        goto exit;
    }

    if (rx_data.data[0] != 'v' || rx_data.data[1] != ' ')
    {
        ret = PLATFORM_EINVAL;
        goto exit;
    }
    
    char *end_pos;
    *data = strtol(rx_data.data + 2, &end_pos, 10);
exit:
    return ret;
}

static platform_err_t get_cmd_s32_packet_send(struct h_motor_acm *me, 
                                char *str, uint32_t len, int32_t *data)
{
    platform_err_t ret = PLATFORM_EOK;
    
    for(uint32_t i = 0; i < RETRANSMISSION_TIMES; i++)
    {
        rcv_queue_reset(me);
        platform_device_write(me->uart_dev, 0, str, len);
        ret = get_cmd_s32_rcv(me, data);
        
        if(PLATFORM_EOK == ret)  break;
    }

    return ret;
}

static platform_err_t set_cmd_packet_send(struct h_motor_acm *me, char *str, uint32_t len)
{
    platform_err_t ret = PLATFORM_EOK;
    
    for(uint32_t i = 0; i < RETRANSMISSION_TIMES; i++)
    {
        rcv_queue_reset(me);
        platform_device_write(me->uart_dev, 0, str, len);
        ret = set_cmd_rcv(me);
        
        if(PLATFORM_EOK == ret)  break;
    }

    return ret;
}

static void  err_cb_check(struct h_motor_acm *me, platform_err_t ret)
{
    if (PLATFORM_EOK != ret)
    {
        log_e("comm err: work_id=%lu, ret=%d",
              me->last_work_id, ret);
        me->status.is_commnucation_err = true;
        osMessageQueueReset(me->work_queue);
        if (me->base.err_cb)
            me->base.err_cb(H_MOTOR_COMMUNICATION_ERR,
                            &me->base);
    }
    else
    {
        me->status.is_commnucation_err = false;
        if (me->base.err_cb)
            me->base.err_cb(H_MOTOR_COMMUNICATION_OK,
                            &me->base);
    }
}

static platform_err_t u32_reg_set(struct h_motor_acm *me,
                                   uint16_t reg_addr,
                                   uint32_t data)
{
    platform_err_t ret;
    char str[30];

    uint32_t len = sprintf(str, "s r0x%x %lu\r",
                           reg_addr, data);
    ret = set_cmd_packet_send(me, str, len);

    return ret;
}

static platform_err_t s32_reg_set(struct h_motor_acm *me,
                                   uint16_t reg_addr,
                                   int32_t data)
{
    platform_err_t ret;
    char str[30];

    uint32_t len = sprintf(str, "s r0x%x %ld\r",
                           reg_addr, data);
    ret = set_cmd_packet_send(me, str, len);

    return ret;
}

static platform_err_t t_cmd_set(struct h_motor_acm *me, uint32_t cmd_id)
{
    platform_err_t ret;
    char str[30];
    
    uint32_t len = sprintf(str, "t %lu\r", cmd_id);
    ret = set_cmd_packet_send(me, str, len);
    
    return ret;
}

static platform_err_t u32_reg_get(struct h_motor_acm *me,
                                   uint16_t reg_addr,
                                   uint32_t *data)
{
    platform_err_t ret;
    char str[30];

    uint32_t len = sprintf(str, "g r0x%x\r", reg_addr);
    ret = get_cmd_u32_packet_send(me, str, len, data);

    return ret;
}

static platform_err_t s32_reg_get(struct h_motor_acm *me,
                                   uint16_t reg_addr,
                                   int32_t *data)
{
    platform_err_t ret;
    char str[30];

    uint32_t len = sprintf(str, "g r0x%x\r", reg_addr);
    ret = get_cmd_s32_packet_send(me, str, len, data);

    return ret;
}

/* ============================================================ *
 *  模块 2：接收线程 - 从 UART 阻塞读字节，喂给协议解析器       *
 * ============================================================ */
/* 独立线程的好处：work_thread 在等响应（osMessageQueueGet
 * rx_queue）时，rx_thread 仍能继续读 UART 把字节喂给解析器，
 * 不会因为命令处理阻塞导致丢字节。 */
static void rx_thread(void *argument)
{
    struct h_motor_acm *me = argument;
    uint8_t data[RX_DATA_SIZE_MAX];

    for (;;)
    {
        uint32_t read_cnt = platform_device_read(me->uart_dev, 0, data,
                                                 RX_DATA_SIZE_MAX);
        if (read_cnt > 0)
        {
            for (uint32_t i = 0; i < read_cnt; i++)
            {
                packet_parser_input(me, data[i]);
            }
        }
    }
}

uint8_t fault_detect_pin_level_get(uint8_t id, void* args)
{
    (void)id;
    struct h_motor_acm *me = args;
    return platform_pin_read(me->fault_detect_pin);
}

static void acm_work_queue_put(struct h_motor_base *me,
                               acm_work_t *work);

void fault_btn_cb(void* args)
{
    struct h_motor_acm *me = container_of(
        args, struct h_motor_acm, fault_detect_btn);
    Button *btn = args;
    PressEvent event = get_button_event(btn);
    if (event == PRESS_DOWN)
    {
        acm_work_t work;
        work.work_id = ACM_FAULT_DIAG;
        acm_work_queue_put(&me->base, &work);
        if (me->base.err_cb)
            me->base.err_cb(H_MOTOR_HW_ERR,
                            &me->base);
        me->status.is_hw_error = true;
    } 
    else
    {
        me->status.is_hw_error = false;
    }
}

static int32_t mm_to_cnt(struct h_motor_acm *me, double mm)
{
    return (int32_t)round(mm * me->encoder_cnt_per_mm);
}

static double cnt_to_mm(struct h_motor_acm *me, int32_t cnt)
{
    return (double)cnt / me->encoder_cnt_per_mm;
}

static void acm_work_queue_put(struct h_motor_base *me, acm_work_t *work)
{   
    osStatus_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    status = osMessageQueuePut(acm->work_queue, work, 0, 0);
    if (status != osOK)
    {
        log_e("queue full: work_id=%lu, count=%lu",
              work->work_id,
              osMessageQueueGetCount(acm->work_queue));
        err_cb_check(acm, PLATFORM_EFULL);
    }
}

static void _motor_stop(struct h_motor_base *me)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    status = t_cmd_set(acm, 0);

#if H_MOTOR_ACM_DEBUG
    log_d("stop: fe_max=%ld cnt (%.1f mm), "
          "cur_max=%d (%.2f A), i2t_max=%d (%.2f%%)",
          acm->max_following_error,
          cnt_to_mm(acm, acm->max_following_error),
          acm->max_motor_current,
          acm->max_motor_current / 100.0,
          acm->max_i2t_sum,
          acm->max_i2t_sum / 100.0);
    acm->max_following_error = 0;
    acm->max_motor_current = 0;
    acm->max_i2t_sum = 0;
#endif /* H_MOTOR_ACM_DEBUG */

    err_cb_check(acm, status);
}

static void motor_stop(struct h_motor_base *me)
{
    acm_work_t work;
    
    work.work_id = ACM_STOP;
    acm_work_queue_put(me, &work);
}


static void _motor_start(struct h_motor_base *me)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    status = t_cmd_set(acm, 1);
    err_cb_check(acm, status);
}

static void motor_start(struct h_motor_base *me)
{
    acm_work_t work;
    
    work.work_id = ACM_START;
    acm_work_queue_put(me, &work);
}

static void _move_velocity_mode(struct h_motor_base *me, enum h_motor_base_move_dir dir)
{
    int32_t data;
    platform_err_t status;
    
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    status = u32_reg_set(acm, 0xc8, 2);
    
    if(H_MOTOR_MOVE_POSITIVE == dir)
    {
        data = 1;
    }
    else
    {
        data = -1;
    }
    status = status || s32_reg_set(acm, 0xca, data);
    status = status || t_cmd_set(acm, 1);
    err_cb_check(acm, status);
}

static void move_velocity_mode(struct h_motor_base *me, enum h_motor_base_move_dir dir)
{
    acm_work_t work;
    
    work.work_id = ACM_MOVE_VELOCITY_MODE;
    work.work_param.s32_data = (int32_t)dir;
    acm_work_queue_put(me, &work);
}

static void _move_relative_mode(struct h_motor_base *me, double move_mm)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;

    status = u32_reg_set(acm, 0xc8, 256);
    status = status || s32_reg_set(acm, 0xca, mm_to_cnt(acm, move_mm));
    status = status || t_cmd_set(acm, 1);
    err_cb_check(acm, status);
}

static void move_relative_mode(struct h_motor_base *me, double move_mm)
{
    acm_work_t work;
    
    work.work_id = ACM_MOVE_RELATIVE_MODE;
    work.work_param.double_data = move_mm;
    acm_work_queue_put(me, &work);
}


static void _move_absolute_mode(struct h_motor_base *me, double pos_mm)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;

    status = u32_reg_set(acm, 0xc8, 0);
    status = status || s32_reg_set(acm, 0xca, mm_to_cnt(acm, pos_mm));
    status = status || t_cmd_set(acm, 1);
    err_cb_check(acm, status);
}

static void move_absolute_mode(struct h_motor_base *me, double pos_mm)
{
    acm_work_t work;
    
    work.work_id = ACM_MOVE_ABSOLUTE_MODE;
    work.work_param.double_data = pos_mm;
    acm_work_queue_put(me, &work);
}

static platform_err_t status_read(struct h_motor_base *me)
{
    platform_err_t status;
    uint32_t data;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    status = u32_reg_get(acm, 0xc9, &data);
    if (PLATFORM_EOK == status)
    {
        acm->status.is_homing_error = data & (0x01 << 11);
        acm->status.is_homing_success = data & (0x01 << 12);
        acm->status.is_homing_state = data & (0x01 << 13);
        acm->status.is_in_motion = data & (0x01 << 15);
    }
    return status;
}

static void _velocity_set(struct h_motor_base *me, int32_t velocity)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    status = s32_reg_set(acm, 0xcb, mm_to_cnt(acm, velocity) * 10);
    status = status || status_read(me);
    if (acm->status.is_in_motion && PLATFORM_EOK == status)
    {
        status = status || t_cmd_set(acm, 1);
    }
    err_cb_check(acm, status);
}

static void velocity_set(struct h_motor_base *me, int32_t velocity)
{
    acm_work_t work;
    
    work.work_id = ACM_VELOCITY_SET;
    work.work_param.s32_data = velocity;
    acm_work_queue_put(me, &work);
}


static void _acce_dece_set(struct h_motor_base *me, uint32_t acce, uint32_t dece)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    int32_t acce_cnt = (int32_t)(mm_to_cnt(acm, acce) / 10);
    int32_t dece_cnt = (int32_t)(mm_to_cnt(acm, dece) / 10);
    status = u32_reg_set(acm, 0xcc, acce_cnt);
    status = status || u32_reg_set(acm, 0xcd, dece_cnt);
    status = status || u32_reg_set(acm, 0xcf, dece_cnt); 
    status = status || status_read(me);
    if (acm->status.is_in_motion && PLATFORM_EOK == status)
    {
        status = status || t_cmd_set(acm, 1);
    }
    err_cb_check(acm, status);
}

static void acce_dece_set(struct h_motor_base *me, uint32_t acce, uint32_t dece)
{
    acm_work_t work;
    
    work.work_id = ACM_ACCE_DECE_SET;
    work.work_param.u32_data[0] = acce;
    work.work_param.u32_data[1] = dece;
    acm_work_queue_put(me, &work);
}

static void manual_mode(struct h_motor_base *me, bool is_manual)
{
    struct h_motor_acm *acm = (struct h_motor_acm *)me;

    int32_t level;

    if (is_manual) level = !CLUTCH_ENABLE_LEVEL;
    else level = CLUTCH_ENABLE_LEVEL;
    
    platform_pin_write(acm->clutch_pin, level);
}

static void _status_get(struct h_motor_base *me)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    status = status_read(me);
    if (acm->base.status_cb && PLATFORM_EOK == status)
    {
        acm->base.status_cb(&acm->status, me);
    }
    err_cb_check(acm, status);
}

static void status_get(struct h_motor_base *me)
{
    acm_work_t work;
    
    work.work_id = ACM_STATUS_GET;
    acm_work_queue_put(me, &work);
}

static bool _status_get_sync(struct h_motor_base *me, 
                              struct h_motor_base_status *status)
{
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    
    if (me == NULL || status == NULL)
    {
        return false;
    }
    
    /* Directly return cached status */
    memcpy(status, &acm->status, sizeof(struct h_motor_base_status));
    return true;
}

static void _cur_pos_get(struct h_motor_base *me)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    int32_t cur_pos;
    status = s32_reg_get(acm, ACM_REG_ACTUAL_POSITION,
                         &cur_pos);

    if (acm->base.cur_pos_cb && PLATFORM_EOK == status)
    {
        double cur_pos_mm = cnt_to_mm(acm, cur_pos);
        acm->base.cur_pos_cb(cur_pos_mm, me);
    }

#if H_MOTOR_ACM_DEBUG
    if (PLATFORM_EOK == status)
    {
        acm->debug_print_cnt++;
        if (acm->debug_print_cnt >= ACM_DEBUG_PRINT_INTERVAL)
        {
            int32_t fe_raw = 0;
            int16_t cur_abs = 0;
            int16_t i2t_val = 0;
            int32_t tmp;

            if (PLATFORM_EOK == s32_reg_get(acm,
                    ACM_REG_FOLLOWING_ERROR, &tmp))
            {
                fe_raw = tmp;
                int32_t abs_err =
                    (tmp < 0) ? -tmp : tmp;
                if (abs_err > acm->max_following_error)
                    acm->max_following_error = abs_err;
            }

            if (PLATFORM_EOK == s32_reg_get(acm,
                    ACM_REG_ACTUAL_MOTOR_CURRENT,
                    &tmp))
            {
                int16_t cur = (int16_t)tmp;
                cur_abs = (cur < 0) ? -cur : cur;
                if (cur_abs > acm->max_motor_current)
                    acm->max_motor_current = cur_abs;
            }

            if (!acm->i2t_reg_unsupported)
            {
                platform_err_t i2t_ret =
                    s32_reg_get(acm,
                        ACM_REG_I2T_RUNNING_SUM, &tmp);
                if (PLATFORM_EOK == i2t_ret)
                {
                    i2t_val = (int16_t)tmp;
                    if (i2t_val > acm->max_i2t_sum)
                        acm->max_i2t_sum = i2t_val;
                }
                else
                {
                    acm->i2t_reg_unsupported = true;
                    log_e("reg 0x%x read failed "
                          "(ret=%d), not supported",
                          ACM_REG_I2T_RUNNING_SUM,
                          i2t_ret);
                }
            }

            acm->debug_print_cnt = 0;
            log_d("[I2T] cur=%d (%.2fA) i2t=%d "
                  "(%.2f%%) fe=%ld",
                  cur_abs, cur_abs / 100.0,
                  i2t_val, i2t_val / 100.0,
                  fe_raw);
        }
    }
#endif /* H_MOTOR_ACM_DEBUG */

    err_cb_check(acm, status);
}

static void cur_pos_get(struct h_motor_base *me)
{
    acm_work_t work;
    
    work.work_id = ACM_CUR_POS_GET;
    acm_work_queue_put(me, &work);
}

static void _enable(struct h_motor_base *me, bool enable)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    uint32_t data;

    if (enable) data = 21;
    else data = 0;
    
    status = u32_reg_set(acm, 0x24, data);
    err_cb_check(acm, status);
}

static void enable(struct h_motor_base *me, bool enable)
{   
    acm_work_t work;
    
    work.work_id = ACM_ENABLE;
    work.work_param.s32_data = (int32_t)enable;
    acm_work_queue_put(me, &work);
}


static void _home(struct h_motor_base *me)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    status = u32_reg_set(acm, 0xc2, 529);
    status = status || t_cmd_set(acm, 2);
    err_cb_check(acm, status);
}

static void home(struct h_motor_base *me)
{
    acm_work_t work;
    
    work.work_id = ACM_HOME;
    acm_work_queue_put(me, &work);
}

static void _home_set(struct h_motor_base *me, double home_offset)
{
    platform_err_t status;
    uint32_t traj_status = 0;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;

    status = u32_reg_get(acm, 0xc9, &traj_status);
    if (PLATFORM_EOK != status)
    {
        err_cb_check(acm, status);
        return;
    }

    bool is_referenced = (traj_status >> 12) & 0x01;
    if (!is_referenced)
    {
        /* Referenced bit is 0 (after power-on/wake-up).
         * Execute homing method 528 to set Referenced bit.
         * Method 528 resets position to 0 instantly,
         * so load position must be written after this. */
        status = u32_reg_set(acm, 0xc2, 528);
        status = status || t_cmd_set(acm, 2);
        /* Wait for Referenced bit to be set before
         * writing load position. */
        for (int i = 0; i < 3; i++)
        {
            osDelay(10);
            u32_reg_get(acm, 0xc9, &traj_status);
            if ((traj_status >> 12) & 0x01)
            {
                break;
            }
        }
    }

    /* Write load position to set correct offset.
     * During normal operation (Referenced=1), only this
     * step executes, avoiding t 2 Referenced clear risk. */
    status = status || s32_reg_set(acm, 0x17,
                -mm_to_cnt(acm, home_offset));
    u32_reg_get(acm, 0xc9, &traj_status);
    log_d("home_set: offset=%.1f, 0xC9=0x%04lX, "
          "Referenced=%lu",
          home_offset, traj_status,
          (traj_status >> 12) & 0x01);
    err_cb_check(acm, status);
}

static void home_set(struct h_motor_base *me, double home_offset)
{
    acm_work_t work;
    
    work.work_id = ACM_HOME_SET;
    work.work_param.double_data = home_offset;
    acm_work_queue_put(me, &work);
}


static void _positive_limit(struct h_motor_base *me, uint32_t positive_limit)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    status = u32_reg_set(acm, 0xb8, mm_to_cnt(acm, positive_limit));
    err_cb_check(acm, status);
}

static void positive_limit(struct h_motor_base *me, uint32_t positive_limit)
{
    acm_work_t work;

    work.work_id = ACM_POSITIVE_LIMIT;
    work.work_param.u32_data[0] = positive_limit;
    acm_work_queue_put(me, &work);
}

static void _negative_limit(struct h_motor_base *me,
                             int32_t negative_limit_mm)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    /* Register 0xb9: Negative Software Limit (signed) */
    status = s32_reg_set(acm, 0xb9,
                         mm_to_cnt(acm, negative_limit_mm));
    err_cb_check(acm, status);
}

static void negative_limit(struct h_motor_base *me,
                            int32_t negative_limit_mm)
{
    acm_work_t work;

    work.work_id = ACM_NEGATIVE_LIMIT;
    work.work_param.s32_data = negative_limit_mm;
    acm_work_queue_put(me, &work);
}

static void _fault_clear(struct h_motor_base *me)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    status = u32_reg_set(acm, ACM_REG_LATCHING_FAULT_STATUS,
                         0xffffffff);
    err_cb_check(acm, status);
}

static void fault_clear(struct h_motor_base *me)
{
    acm_work_t work;
    
    work.work_id = ACM_FAULT_CLEAR;
    acm_work_queue_put(me, &work);
}

typedef struct
{
    uint32_t bit_mask;
    const char *name;
} reg_bit_desc_t;

/* Event Status Register (0xA0) bit definitions
 * per ACM 伺服厂家 Parameter Dictionary */
static const reg_bit_desc_t evt_bits[] =
{
    {(1U << 0),  "ShortCircuit"},
    {(1U << 1),  "AmpOverTemp"},
    {(1U << 2),  "OverVoltage"},
    {(1U << 3),  "UnderVoltage"},
    {(1U << 4),  "MotorTemp"},
    {(1U << 5),  "EncoderErr"},
    {(1U << 6),  "PhasingErr"},
    {(1U << 7),  "CurLimited(I2T)"},
    {(1U << 8),  "VoltLimited"},
    {(1U << 9),  "PosLimitSw"},
    {(1U << 10), "NegLimitSw"},
    {(1U << 11), "EnableInactive"},
    {(1U << 12), "SwDisabled"},
    {(1U << 13), "StoppingMotor"},
    {(1U << 14), "BrakeActive"},
    {(1U << 15), "PWMDisabled"},
    {(1U << 16), "PosSoftLimit"},
    {(1U << 17), "NegSoftLimit"},
    {(1U << 18), "TrackingErr"},
    {(1U << 19), "TrackingWarn"},
    {(1U << 20), "AmplReset"},
    {(1U << 21), "PosWrapped"},
    {(1U << 22), "LatchedFault"},
    {(1U << 23), "CurLimitActive"},
    {(1U << 24), "BrakeByI2T"},
    {(1U << 27), "PhaseInit"},
    {(1U << 28), "VelWindow"},
    {(1U << 30), "CmdFault"},
};

#define EVT_BITS_NUM \
    (sizeof(evt_bits) / sizeof(evt_bits[0]))

/* Latching Fault Status (0xA4) / Fault Mask (0xA7)
 * bit definitions per ACM 伺服厂家 Parameter Dictionary.
 * NOTE: different bit layout from Event Status! */
static const reg_bit_desc_t fault_bits[] =
{
    {(1U << 0),  "FlashCRC"},
    {(1U << 1),  "AmpInternalErr"},
    {(1U << 2),  "ShortCircuit"},
    {(1U << 3),  "AmpOverTemp"},
    {(1U << 4),  "MotorOverTemp"},
    {(1U << 5),  "OverVoltage"},
    {(1U << 6),  "UnderVoltage"},
    {(1U << 7),  "FeedbackFault"},
    {(1U << 8),  "PhasingErr"},
    {(1U << 9),  "TrackingErr(FE)"},
    {(1U << 10), "I2T_CurLimit"},
    {(1U << 11), "FPGA_Failure"},
    {(1U << 12), "CmdInputLost"},
};

#define FAULT_BITS_NUM \
    (sizeof(fault_bits) / sizeof(fault_bits[0]))

static void _fault_diag(struct h_motor_base *me)
{
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    uint32_t evt_reg = 0;
    uint32_t latched_evt = 0;
    uint32_t fault_reg = 0;
    uint32_t mask_reg = 0;
    int32_t fe_raw = 0;
    int32_t cur_raw = 0;
    int32_t i2t_raw = 0;
    uint32_t i;

    platform_err_t ret;

    u32_reg_get(acm, ACM_REG_EVENT_STATUS, &evt_reg);
    u32_reg_get(acm, ACM_REG_LATCHED_EVENT_STATUS,
                &latched_evt);
    u32_reg_get(acm, ACM_REG_LATCHING_FAULT_STATUS,
                &fault_reg);
    u32_reg_get(acm, ACM_REG_FAULT_MASK, &mask_reg);
    s32_reg_get(acm, ACM_REG_FOLLOWING_ERROR, &fe_raw);
    s32_reg_get(acm, ACM_REG_ACTUAL_MOTOR_CURRENT,
                &cur_raw);
    ret = s32_reg_get(acm, ACM_REG_I2T_RUNNING_SUM,
                      &i2t_raw);

    log_e("=== FAULT DIAG ===");
    log_e("EvtStat=0x%08lx LatchEvt=0x%08lx "
          "FaultStat=0x%08lx Mask=0x%08lx",
          evt_reg, latched_evt, fault_reg, mask_reg);

    if (PLATFORM_EOK != ret)
    {
        log_e("reg 0x%x read failed (ret=%d), "
              "not supported by drive",
              ACM_REG_I2T_RUNNING_SUM, ret);
    }

    log_e("cur=%d (%.2fA) i2t=%d (%.2f%%) fe=%ld",
          (int16_t)cur_raw,
          ((cur_raw < 0) ? -cur_raw : cur_raw) / 100.0,
          (int16_t)i2t_raw, i2t_raw / 100.0,
          fe_raw);

    for (i = 0; i < EVT_BITS_NUM; i++)
    {
        if (latched_evt & evt_bits[i].bit_mask)
        {
            log_e("LatchEvt[0xA1]: %s",
                  evt_bits[i].name);
        }
    }

    for (i = 0; i < FAULT_BITS_NUM; i++)
    {
        if (fault_reg & fault_bits[i].bit_mask)
        {
            log_e("FaultStat[0xA4]: %s",
                  fault_bits[i].name);
        }
    }

    for (i = 0; i < EVT_BITS_NUM; i++)
    {
        if (evt_reg & evt_bits[i].bit_mask)
        {
            log_e("EvtStat[0xA0]: %s",
                  evt_bits[i].name);
        }
    }

    if (fault_reg == 0 && latched_evt == 0
        && evt_reg == 0)
    {
        log_e("No fault/event bits set");
    }

    log_e("=== FAULT DIAG END ===");
}

static void _following_err_enable(struct h_motor_base *me,
                                   bool enable)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    uint32_t reg_data;

    status = u32_reg_get(acm, ACM_REG_FAULT_MASK,
                         &reg_data);
    if (enable)
        reg_data |= ACM_FAULT_MASK_FOLLOWING_ERR_BIT;
    else
        reg_data &= ~ACM_FAULT_MASK_FOLLOWING_ERR_BIT;
    status = status || u32_reg_set(acm, ACM_REG_FAULT_MASK,
                                   reg_data);
    err_cb_check(acm, status);
}

static void following_err_enable(struct h_motor_base *me, bool enable)
{
    acm_work_t work;
    
    work.work_id = ACM_FOLLOWING_ERROR_ENABLE;
    work.work_param.s32_data = (int32_t)enable;
    acm_work_queue_put(me, &work);
}

static void _following_err_limit_set(struct h_motor_base *me,
                                      double limit_mm)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    int32_t limit_cnt = mm_to_cnt(acm, limit_mm);
    status = s32_reg_set(acm, 0xBA, limit_cnt);
    err_cb_check(acm, status);
}

static void following_err_limit_set(struct h_motor_base *me,
                                     double limit_mm)
{
    acm_work_t work;

    work.work_id = ACM_FOLLOWING_ERR_LIMIT_SET;
    work.work_param.double_data = limit_mm;
    acm_work_queue_put(me, &work);
}

static void _encoder_ratio_set(struct h_motor_base *me, double cnt_per_mm)
{
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    if (cnt_per_mm > 0.0)
    {
        acm->encoder_cnt_per_mm = cnt_per_mm;
    }
}

static void encoder_ratio_set(struct h_motor_base *me, double cnt_per_mm)
{
    acm_work_t work;
    
    work.work_id = ACM_ENCODER_RATIO_SET;
    work.work_param.double_data = cnt_per_mm;
    acm_work_queue_put(me, &work);
}

static void _i2t_fault_enable(struct h_motor_base *me,
                               bool enable)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    uint32_t reg_data;

    status = u32_reg_get(acm, ACM_REG_FAULT_MASK,
                         &reg_data);
    if (enable)
        reg_data |= ACM_FAULT_MASK_I2T_BIT;
    else
        reg_data &= ~ACM_FAULT_MASK_I2T_BIT;
    status = status || u32_reg_set(acm, ACM_REG_FAULT_MASK,
                                   reg_data);
    err_cb_check(acm, status);
}

static void i2t_fault_enable(struct h_motor_base *me,
                              bool enable)
{
    acm_work_t work;

    work.work_id = ACM_I2T_FAULT_ENABLE;
    work.work_param.s32_data = (int32_t)enable;
    acm_work_queue_put(me, &work);
}

static void _i2t_peak_current_set(struct h_motor_base *me,
                                   uint16_t current_001a)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    status = u32_reg_set(acm,
                         ACM_REG_PEAK_CURRENT_LIMIT,
                         (uint32_t)current_001a);
    err_cb_check(acm, status);
}

static void i2t_peak_current_set(struct h_motor_base *me,
                                  uint16_t current_001a)
{
    acm_work_t work;

    work.work_id = ACM_I2T_PEAK_CURRENT_SET;
    work.work_param.u32_data[0] = (uint32_t)current_001a;
    acm_work_queue_put(me, &work);
}

static void _i2t_continuous_current_set(
    struct h_motor_base *me, uint16_t current_001a)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    status = u32_reg_set(acm,
                         ACM_REG_CONTINUOUS_CURRENT_LIMIT,
                         (uint32_t)current_001a);
    err_cb_check(acm, status);
}

static void i2t_continuous_current_set(
    struct h_motor_base *me, uint16_t current_001a)
{
    acm_work_t work;

    work.work_id = ACM_I2T_CONTINUOUS_CURRENT_SET;
    work.work_param.u32_data[0] = (uint32_t)current_001a;
    acm_work_queue_put(me, &work);
}

static void _i2t_peak_time_set(struct h_motor_base *me,
                                uint16_t time_ms)
{
    platform_err_t status;
    struct h_motor_acm *acm = (struct h_motor_acm *)me;
    status = u32_reg_set(acm,
                         ACM_REG_PEAK_CURRENT_TIME,
                         (uint32_t)time_ms);
    err_cb_check(acm, status);
}

static void i2t_peak_time_set(struct h_motor_base *me,
                               uint16_t time_ms)
{
    acm_work_t work;

    work.work_id = ACM_I2T_PEAK_TIME_SET;
    work.work_param.u32_data[0] = (uint32_t)time_ms;
    acm_work_queue_put(me, &work);
}

/* 静态 ops 表: const 落 .rodata 全程序共享。所有 struct h_motor_acm
 * 实例 me->ops 都指向这一份。 */
static const struct h_motor_base_ops ops =
{
    .motor_stop = motor_stop,
    .motor_start = motor_start,
    .move_velocity_mode = move_velocity_mode,
    .move_relative_mode = move_relative_mode,
    .move_absolute_mode = move_absolute_mode,
    .velocity_set = velocity_set,
    .acce_dece_set = acce_dece_set,
    .manual_mode = manual_mode,
    .status_get = status_get,
    .status_get_sync = _status_get_sync,
    .cur_pos_get = cur_pos_get,
    .enable = enable,
    .home = home,
    .home_set = home_set,
    .positive_limit = positive_limit,
    .negative_limit = negative_limit,
    .fault_clear = fault_clear,
    .following_err_enable = following_err_enable,
    .following_err_limit_set = following_err_limit_set,
    .encoder_ratio_set = encoder_ratio_set,
    .i2t_fault_enable = i2t_fault_enable,
    .i2t_peak_current_set = i2t_peak_current_set,
    .i2t_continuous_current_set = i2t_continuous_current_set,
    .i2t_peak_time_set = i2t_peak_time_set,
};

static void work_thread(void *argument)
{
    struct h_motor_acm *me = argument;
    acm_work_t work;
    for(;;)
    {
        osMessageQueueGet(me->work_queue, &work, NULL, osWaitForever);
        me->last_work_id = work.work_id;
        switch(work.work_id)
        {
        case ACM_STOP:
        {
            _motor_stop((struct h_motor_base *)me);
            break;
        }
        case ACM_START:
        {
            _motor_start((struct h_motor_base *)me);
            break;
        }
        case ACM_MOVE_VELOCITY_MODE:
        {
            _move_velocity_mode((struct h_motor_base *)me, 
                        (enum h_motor_base_move_dir)work.work_param.s32_data);
            break;
        }
        case ACM_MOVE_RELATIVE_MODE:
        {
            _move_relative_mode((struct h_motor_base *)me, 
                        work.work_param.double_data);
            break;
        }
        case ACM_MOVE_ABSOLUTE_MODE:
        {
            _move_absolute_mode((struct h_motor_base *)me, 
                        work.work_param.double_data);
            break;
        }
        case ACM_VELOCITY_SET:
        {
            _velocity_set((struct h_motor_base *)me, work.work_param.s32_data);
            break;
        }
        case ACM_ACCE_DECE_SET:
        {
            _acce_dece_set((struct h_motor_base *)me, 
                        work.work_param.u32_data[0], 
                        work.work_param.u32_data[1]);
            break;
        }
        case ACM_STATUS_GET:
        {
            _status_get((struct h_motor_base *)me);  
            break;
        }
        case ACM_CUR_POS_GET:
        {
            _cur_pos_get((struct h_motor_base *)me);
            break;
        }
        case ACM_ENABLE:
        {
            _enable((struct h_motor_base *)me, (bool)work.work_param.s32_data);
            break;
        }
        case ACM_HOME:
        {
            _home((struct h_motor_base *)me);
            break;
        }
        case ACM_HOME_SET:
        {
            _home_set((struct h_motor_base *)me, work.work_param.double_data);
            break;
        }
        case ACM_POSITIVE_LIMIT:
        {
            _positive_limit((struct h_motor_base *)me,
                        work.work_param.u32_data[0]);
            break;
        }
        case ACM_NEGATIVE_LIMIT:
        {
            _negative_limit((struct h_motor_base *)me,
                        work.work_param.s32_data);
            break;
        }
        case ACM_FAULT_CLEAR:
        {
            _fault_clear((struct h_motor_base *)me);
            break;
        }
        case ACM_FOLLOWING_ERROR_ENABLE:
        {
            _following_err_enable((struct h_motor_base *)me, 
                        (bool)work.work_param.s32_data);
            break;
        }
        case ACM_FOLLOWING_ERR_LIMIT_SET:
        {
            _following_err_limit_set((struct h_motor_base *)me,
                        work.work_param.double_data);
            break;
        }
        case ACM_ENCODER_RATIO_SET:
        {
            _encoder_ratio_set((struct h_motor_base *)me, 
                        work.work_param.double_data);
            break;
        }
        case ACM_I2T_FAULT_ENABLE:
        {
            _i2t_fault_enable((struct h_motor_base *)me,
                        (bool)work.work_param.s32_data);
            break;
        }
        case ACM_I2T_PEAK_CURRENT_SET:
        {
            _i2t_peak_current_set(
                (struct h_motor_base *)me,
                (uint16_t)work.work_param.u32_data[0]);
            break;
        }
        case ACM_I2T_CONTINUOUS_CURRENT_SET:
        {
            _i2t_continuous_current_set(
                (struct h_motor_base *)me,
                (uint16_t)work.work_param.u32_data[0]);
            break;
        }
        case ACM_I2T_PEAK_TIME_SET:
        {
            _i2t_peak_time_set(
                (struct h_motor_base *)me,
                (uint16_t)work.work_param.u32_data[0]);
            break;
        }
        case ACM_FAULT_DIAG:
        {
            _fault_diag((struct h_motor_base *)me);
            break;
        }
        default:
        {
            break;
        }
        }
    }
}

void h_motor_acm_init(struct h_motor_acm *me, struct h_motor_acm_init_param *init_param)
{
    platform_err_t ret;
    
    osThreadAttr_t work_thread_attr =
    {
        .name = "acm_work",
        .attr_bits = osThreadDetached,
        .priority = osPriorityAboveNormal,
        .stack_size = 2048,
    };

    osThreadAttr_t rx_thread_attr =
    {
        .name = "acm_rx",
        .attr_bits = osThreadDetached,
        .priority = osPriorityAboveNormal,
        .stack_size = 1024,
    };
    
    memset(me, 0, sizeof(struct h_motor_acm));
    h_motor_base_init(&me->base);

    me->encoder_cnt_per_mm = DEFAULT_ENCODER_CNT_PER_MM;
    me->base.ops = &ops;
    me->uart_dev = platform_device_find(init_param->uart_name);
    platform_assert(me->uart_dev);
    ret = platform_device_open(me->uart_dev);
    platform_assert(PLATFORM_EOK == ret);

    me->clutch_pin = platform_pin_get(init_param->clutch_pin_name);
    platform_assert(me->clutch_pin >= 0);
    platform_pin_write(me->clutch_pin, !CLUTCH_ENABLE_LEVEL);
    platform_pin_mode(me->clutch_pin, PIN_MODE_OUTPUT);

    me->rev_en_pin = platform_pin_get(init_param->rev_en_pin_name);
    platform_assert(me->rev_en_pin >= 0);
    platform_pin_write(me->rev_en_pin, REV_EN_PIN_DEFAULT_LEVEL);
    platform_pin_mode(me->rev_en_pin, PIN_MODE_OUTPUT);

    me->en_pin = platform_pin_get(init_param->en_pin_name);
    platform_assert(me->en_pin >= 0);
    platform_pin_write(me->en_pin, EN_PIN_DEFAULT_LEVEL);
    platform_pin_mode(me->en_pin, PIN_MODE_OUTPUT);

    me->fwd_pin = platform_pin_get(init_param->fwd_pin_name);
    platform_assert(me->fwd_pin >= 0);
    platform_pin_write(me->fwd_pin, FWD_PIN_DEFAULT_LEVEL);
    platform_pin_mode(me->fwd_pin, PIN_MODE_OUTPUT);

    me->fault_detect_pin = platform_pin_get(init_param->fault_detect_pin_name);
    platform_assert(me->fault_detect_pin >= 0);
    platform_pin_mode(me->fault_detect_pin, PIN_MODE_INPUT);

    if(platform_pin_read(me->fault_detect_pin) == FAULT_DETECT_PIN_FAULT_LEVEL)
    {
        me->status.is_hw_error = true;
    }
    
    me->work_queue = osMessageQueueNew(WORK_QUEUE_ELEM_NUM, sizeof(acm_work_t),
                                 NULL);
    platform_assert(me->work_queue != NULL);

    me->rx_queue = osMessageQueueNew(RX_QUEUE_ELEM_NUM, sizeof(rx_data_t),
                                 NULL);
    platform_assert(me->rx_queue != NULL);

    me->work_thread = osThreadNew(work_thread, me, &work_thread_attr);
    platform_assert(me->work_thread != NULL);
    
    me->rx_thread = osThreadNew(rx_thread, me, &rx_thread_attr);
    platform_assert(me->rx_thread != NULL);

    button_init(&me->fault_detect_btn, fault_detect_pin_level_get,
        FAULT_DETECT_PIN_FAULT_LEVEL, 0, me);
    button_attach(&me->fault_detect_btn, PRESS_DOWN, fault_btn_cb);
    button_attach(&me->fault_detect_btn, PRESS_UP, fault_btn_cb);
    button_start(&me->fault_detect_btn);
}

