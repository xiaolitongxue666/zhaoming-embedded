/**
  ******************************************************************************
  * @file    h_motor_acm.h
  * @brief   ACM 伺服电机子类（h_motor_base 的具体实现）。
  *          继承 h_motor_base，把 24 个虚方法落到"通过 UART 发
  *          厂家私有 ASCII 命令、阻塞读响应、按需回调"这套实现上。
  *
  *          关键设计：
  *            - 工作线程 work_thread + 命令队列 work_queue
  *              把 24 个"看似阻塞"的 ops 接口改写成非阻塞调用。
  *            - 接收线程 rx_thread + 接收队列 rx_queue
  *              专门收串口字节流，解帧后丢进 rx_queue 给 work_thread 取。
  *              两线程独立避免死锁（命令线程等响应时不会卡死接收线程）。
  *            - 故障检测引脚通过 MultiButton 库当成"按键事件"处理，
  *              得到"短按 / 长按 / 双击"统一事件接口。
  *            - 性能监控字段 (max_following_error / max_motor_current
  *              / max_i2t_sum) 持续记录运行期间峰值，故障复盘金贵信息。
  *            - 兼容性字段 i2t_reg_unsupported 适配同芯片不同固件版本：
  *              老固件没有 I²t 寄存器，标志位置 true 后相关 ops 自动 no-op。
  ******************************************************************************
  */

#ifndef __H_MOTOR_ACM_H
#define __H_MOTOR_ACM_H

#include "h_motor_base.h"
#include "platform_device.h"
#include "multi_button.h"

/* 单条响应帧的最大字节数（ACM ASCII 协议的最长 "v <int32>\r" 之类）。 */
#define RX_DATA_SIZE_MAX                                (30)

/* 构造参数。全是字符串引脚名，具体引脚号由 platform_pin_get(name)
 * 在构造函数内部查表得到。换 MCU 只改 board 层引脚表，子类代码不动。 */
typedef struct
{
    const char *uart_name;             /* 走 ACM 命令的 UART 设备名 */
    const char *clutch_pin_name;       /* 离合器使能引脚            */
    const char *rev_en_pin_name;       /* 反向使能                  */
    const char *en_pin_name;           /* 总使能                    */
    const char *fwd_pin_name;          /* 正向使能                  */
    const char *fault_detect_pin_name; /* 故障检测输入引脚          */
} h_motor_acm_init_param_t;

typedef struct
{
    uint8_t buf[RX_DATA_SIZE_MAX];
    uint8_t pos;
} h_motor_acm_rx_buf_t;

/* 子类对象。base 字段必须放第一字段，向上转型零代价。
 * 字段名按教学版/全仓约定统一叫 base，避免读者切换上下文。 */
typedef struct
{
    h_motor_base_t base;             /* 必须在第一字段             */

    /* —— 硬件句柄 —— */
    platform_device_t  uart_dev;     /* ACM 命令通信 UART          */
    int32_t clutch_pin;              /* 离合器引脚号               */
    int32_t rev_en_pin;              /* 反向使能引脚号             */
    int32_t en_pin;                  /* 总使能引脚号               */
    int32_t fwd_pin;                 /* 正向使能引脚号             */
    int32_t fault_detect_pin;        /* 故障检测引脚号             */

    /* —— 双队列 + 双线程 —— */
    /* work_queue / work_thread:
     *   应用层下来的 24 类命令排队等被处理，串行执行，
     *   保证一条命令做完（含等响应）才处理下一条。
     * rx_queue / rx_thread:
     *   专门读 UART 字节流、按 ACM 协议组帧、把完整帧丢进 rx_queue。
     *   work_thread 处理某条命令时阻塞读 rx_queue 拿响应。
     * 两套独立避免死锁。 */
    osMessageQueueId_t work_queue;
    osMessageQueueId_t rx_queue;
    osThreadId_t work_thread;
    osThreadId_t rx_thread;

    /* 接收解帧用的字节缓冲。rx_thread 写入，rx_thread 读出。 */
    h_motor_acm_rx_buf_t rx_buf;

    /* 上一字节时间戳。两字节间隔超 30ms 视为新帧起点。 */
    uint32_t rcv_last_byte_ticks;

    /* 故障引脚走 MultiButton 库做去抖 + 边沿检测。把"故障"
     * 当作一个"按键事件"统一处理。 */
    Button fault_detect_btn;

    /* 当前缓存的状态（按需更新，配合 status_get_sync 立刻返回）。 */
    h_motor_base_status_t status;

    /* —— 标定 / 性能监控字段 —— */
    double  encoder_cnt_per_mm;   /* 编码器脉冲数 / 毫米的标定比例 */
    int32_t max_following_error;  /* 运行期间观察到的最大跟随误差(cnt) */
    int16_t max_motor_current;    /* 运行期间最大电机电流（单位 0.01A） */
    int16_t max_i2t_sum;          /* 运行期间最大 I²t 累积（0.01%） */

    /* —— 兼容性 + 调试 —— */
    /* 老固件不支持 0x132 寄存器，构造期间读失败就置 true，
     * 后续 i2t_xxx 系列虚方法自动 no-op。 */
    bool     i2t_reg_unsupported;

    /* 周期调试打印分频计数（生产固件下 H_MOTOR_ACM_DEBUG=0 关闭）。 */
    uint8_t  debug_print_cnt;

    /* 命令排序号。每条命令进 work_queue 时打一个递增 id，
     * 配合错误日志精确定位"哪条命令丢了响应"。 */
    uint32_t last_work_id;
} h_motor_acm_t;


/* 构造函数：注册 ops、查表 6 路 GPIO、open UART、创建两套
 * 队列 + 两条线程、初始化故障检测按键、读一次 I²t 寄存器探测
 * 老固件兼容性。 */
void h_motor_acm_init(h_motor_acm_t *me,
                      h_motor_acm_init_param_t *init_param);

#endif /* __H_MOTOR_ACM_H */
