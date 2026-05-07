/* SPDX-License-Identifier: MIT */
/*
 * environment_export.h - 环境层句柄统一导出 + init/exit 入口.
 *
 * 应用层 #include "environment_cfg/environment_export.h" 就能拿到所有
 * 设备句柄, 不需要逐个 extern. 子类完整类型 (struct led_gpio /
 * struct led_pwm / struct led_i2c) 应用层看不到, 只看到基类指针.
 *
 * Linux 用户态没有 Linux 内核 initcall, 也不要套自己的 initcall 框架: main
 * 显式调一次 environment_init() 装配, 退出前 environment_exit() 释放. 应用
 * 层调用形态简单直接, 控制流可见.
 */

#ifndef ENVIRONMENT_CFG_ENVIRONMENT_EXPORT_H_
#define ENVIRONMENT_CFG_ENVIRONMENT_EXPORT_H_

#include "drivers/led/led_base.h"

/* 4 颗 LED, 演示 3 种子类混搭:
 *   led_status   GPIO LED   板上状态指示, 高电平点亮 (BCM 17)
 *   led_dimmer   PWM  LED   亮度可调 (pwmchip0/pwm0, 1 kHz)
 *   led_panel    I2C  LED   面板控制器 (i2c-1, addr 0x3C, reg 0x00)
 *   led_alarm    GPIO LED   告警, 高电平点亮 (BCM 22)
 */
extern struct led_base *led_status;
extern struct led_base *led_dimmer;
extern struct led_base *led_panel;
extern struct led_base *led_alarm;

/* 启动期装配 4 个 LED 实例. 任一颗失败返回非零 (上层选择继续或退出). */
int  environment_init(void);

/* 退出前释放. 关 fd, release libgpiod line. */
void environment_exit(void);

#endif /* ENVIRONMENT_CFG_ENVIRONMENT_EXPORT_H_ */
