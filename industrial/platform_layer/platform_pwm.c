/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_pwm.c
  * @brief   PWM 抽象层实现 —— 4 个对外 API 一律转 ops 表分发.
  *
  * @details 见 platform_pwm.h. PWM 跟 UART/I2C/SPI 同套两层继承结构, 但 ops
  *          表更窄: 只有 4 个虚函数 (set / set_pulse / enable / disable),
  *          每家 MCU 后端 (platform_hw_pwm_<chip>.c) 填这 4 个就完事.
  *
  *          这里面没有 mutex / ref_count, 因为 PWM 通常是"单方向输出, 多
  *          通道独立寄存器", 上层调用方各管各的通道, 没有并发争用问题.
  *          需要的话可以后期在本文件套一层锁.
  *
  *          注意工业项目里的 PWM 老接口是把所有动作都套进 control(cmd, args)
  *          一个总入口的形式, 这一版改成每个动作一个具体 op (set/set_pulse/
  *          enable/disable), 更直观, 也方便上层在编译期就被链接器抓到
  *          "ops 漏填了哪个虚函数".
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "platform_pwm.h"
#include "platform_assert.h"

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  把后端实现好的 platform_pwm_dev 注册进 platform_device 表.
  *         上层用 platform_device_find(name) 反查得到的 platform_device *,
  *         可以直接强转回 platform_device_pwm_t (parent 是第一字段).
  *
  * @param  dev       已经填好 ops 的 PWM 设备实例.
  * @param  name      设备名.
  * @param  user_data 透传给后端的私有数据 (典型: TIMx 句柄 + 通道映射表).
  * @retval 见 platform_err_t.
  */
platform_err_t platform_hw_pwm_register(platform_device_pwm_t dev,
                                        const char *name,
                                        void *user_data)
{
    platform_assert(dev != NULL);
    platform_assert(dev->ops != NULL);

    /* 注意: dev->ops 必须由调用方在 register 之前填好,
     * 这里做个断言提示后端不要漏填. */

    dev->parent.ops       = NULL;       /* PWM 不走通用 read/write/control */
    dev->parent.user_data = user_data;

    return platform_device_register(&dev->parent, name,
                                    PLATFORM_DEVICE_FLAG_STANDALONE);
}

/**
  * @brief  一次性配 period + pulse. period 是 PWM 周期, pulse 是高电平占空数.
  *         pulse / period = 占空比. 单位 (tick/ns/us) 由具体后端约定.
  */
platform_err_t platform_pwm_set(platform_device_pwm_t dev,
                                int32_t channel,
                                uint32_t period,
                                uint32_t pulse)
{
    if (NULL == dev) return PLATFORM_EIO;

    if (NULL == dev->ops || NULL == dev->ops->set)
    {
        /* 后端没填 set, 视为不支持 */
        return PLATFORM_ENOSYS;
    }

    return dev->ops->set(dev, channel, period, pulse);
}

/**
  * @brief  仅更新 pulse, 不改 period. 运动控制里调速最常用 (period 固定,
  *         每个控制周期只动 pulse).
  */
platform_err_t platform_pwm_set_pulse(platform_device_pwm_t dev,
                                      int32_t channel,
                                      uint32_t pulse)
{
    if (NULL == dev) return PLATFORM_EIO;

    if (NULL == dev->ops || NULL == dev->ops->set_pulse)
    {
        return PLATFORM_ENOSYS;
    }

    return dev->ops->set_pulse(dev, channel, pulse);
}

/**
  * @brief  使能某个通道的 PWM 输出.
  */
platform_err_t platform_pwm_enable(platform_device_pwm_t dev,
                                   int32_t channel)
{
    if (NULL == dev) return PLATFORM_EIO;

    if (NULL == dev->ops || NULL == dev->ops->enable)
    {
        return PLATFORM_ENOSYS;
    }

    return dev->ops->enable(dev, channel);
}

/**
  * @brief  失能某个通道的 PWM 输出.
  */
platform_err_t platform_pwm_disable(platform_device_pwm_t dev,
                                    int32_t channel)
{
    if (NULL == dev) return PLATFORM_EIO;

    if (NULL == dev->ops || NULL == dev->ops->disable)
    {
        return PLATFORM_ENOSYS;
    }

    return dev->ops->disable(dev, channel);
}

/******************** END OF FILE ******************END OF FILE****/
