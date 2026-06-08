/* SPDX-License-Identifier: MIT */
/**
 * @file  platform_init.h
 * @brief platform 层 ops 表注册入口 (PC 端)
 *
 * @details
 * 把 PC 后端的 platform_pwm / platform_i2c ops 表注册进 dispatcher,
 * 给所有外设共用. 不归 LED 单独管 -- 真实工程一块板上的 sensor /
 * uart / motor 也要走这套 platform 层, 所以拆出来.
 *
 * STM32 端这一步在 platform/arch/stm32/{pin,pwm,i2c}_board.c 的
 * platform_hw_xxx_init 里做; NXP 端在 arch/nxp/ 同款文件里做; PC
 * 端拆成 platform_pwm_pc.c + platform_i2c_pc.c 两份小文件, 这里
 * platform_init() 把两个一次调完.
 *
 * 启动顺序 (main): platform_init() -> led_board_init() -> 应用代码.
 * 注意: 层号自上而下 1=应用 … 4=Platform, 层号不等于执行顺序; Platform
 * 虽是最底层, 但必须最先 platform_init() 给 dispatcher 注册 ops.
 *
 * Dispatcher: ../platform/platform_pwm.c / platform_i2c.c 存 _g_ops / _g_bus,
 * 对外 platform_pwm_enable() 等固定 API, 对内转发到 platform_*_pc.c 注册
 * 的后端. 详见 pc/README.md.
 *
 * platform_gpio (common/platform_pc.c) 是 ch01-ch14 一路用下来的
 * 简版 (直接定义 platform_gpio_init / write 4 个函数, 不走 ops 表
 * + register), 教学渐进的产物. ch15 章节正文 § 15.7 / § 15.11 说明
 * 真实工业项目这一层也升 ops 表 + register, 见 platform_pin.h.
 */

#ifndef PLATFORM_INIT_H
#define PLATFORM_INIT_H

int platform_init(void);

#endif /* PLATFORM_INIT_H */
