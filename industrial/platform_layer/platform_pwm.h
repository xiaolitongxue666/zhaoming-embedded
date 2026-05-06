/**
  ******************************************************************************
  * @file    platform_pwm.h
  * @brief   PWM 抽象层接口 —— Platform 层 PWM 设备的 ops 表与对外 API。
  *          上层驱动（例如 H 桥直流电机调速）只调本头文件里声明的
  *          platform_pwm_xxx 接口，每家 MCU 的 PWM 外设差异由具体后端
  *          (platform_hw_pwm_<chip>.c) 在 platform_pwm_ops_t 里吸收掉。
  *
  *          典型用法：
  *
  *              dev = (platform_device_pwm_t)
  *                    platform_device_find("pwm_tim1");
  *              platform_pwm_set(dev, channel, period, pulse);
  *              platform_pwm_enable(dev, channel);
  *              ...
  *              platform_pwm_set_pulse(dev, channel, new_pulse);
  *              ...
  *              platform_pwm_disable(dev, channel);
  *
  *          period / pulse 的单位由具体后端约定（一般是 PWM 计数器 tick），
  *          上层调用方不直接接触寄存器。
  ******************************************************************************
  */

#ifndef __PLATFORM_PWM_H_
#define __PLATFORM_PWM_H_

#include "platform_device.h"

/* PWM 设备句柄。指向 platform_pwm_dev_t 的不透明指针。 */
typedef struct platform_pwm_dev *platform_device_pwm_t;

/* ----------------------- ops 表 ------------------------------------------ */
/* 每家 MCU 的 PWM 后端实现一份 platform_pwm_ops_t，挂到注册接口上。 */
typedef struct platform_pwm_ops
{
    /* 设置 period 和 pulse（一次性配完）。period 是 PWM 周期，
     * pulse 是高电平占空数。pulse / period = 占空比。 */
    platform_err_t (*set)(platform_device_pwm_t dev,
                          int32_t channel,
                          uint32_t period,
                          uint32_t pulse);

    /* 单独更新占空，不动 period。运动控制里加减速时
     * period 固定不变，只需要每个 tick 更新 pulse。 */
    platform_err_t (*set_pulse)(platform_device_pwm_t dev,
                                int32_t channel,
                                uint32_t pulse);

    /* 使能/失能某个通道的 PWM 输出。 */
    platform_err_t (*enable)(platform_device_pwm_t dev,
                             int32_t channel);
    platform_err_t (*disable)(platform_device_pwm_t dev,
                              int32_t channel);
} platform_pwm_ops_t;

/* ----------------------- 设备结构 ---------------------------------------- */
/* 第一字段 parent 嵌入通用 platform_device，跟 UART/I²C/SPI 同套两层
 * 继承结构（见 ch12 / 第 20 章 20.6 节）。 */
struct platform_pwm_dev
{
    struct platform_device parent;
    const platform_pwm_ops_t *ops;
};

/* ----------------------- 对外 API ---------------------------------------- */
/* 上层驱动调这一组函数即可，不直接访问 ops 表。 */
platform_err_t platform_pwm_set(platform_device_pwm_t dev,
                                int32_t channel,
                                uint32_t period,
                                uint32_t pulse);

platform_err_t platform_pwm_set_pulse(platform_device_pwm_t dev,
                                      int32_t channel,
                                      uint32_t pulse);

platform_err_t platform_pwm_enable(platform_device_pwm_t dev,
                                   int32_t channel);

platform_err_t platform_pwm_disable(platform_device_pwm_t dev,
                                    int32_t channel);

/* 后端注册：MCU 厂商 HAL 适配代码上电时调一次。 */
platform_err_t platform_hw_pwm_register(platform_device_pwm_t dev,
                                        const char *name,
                                        void *user_data);

#endif /* __PLATFORM_PWM_H_ */

/******************** END OF FILE ******************END OF FILE****/
