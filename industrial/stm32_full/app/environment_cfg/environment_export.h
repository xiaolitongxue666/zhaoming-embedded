/* SPDX-License-Identifier: MIT */
/*
 * environment_export.h - 环境层句柄统一导出.
 *
 * 应用层 #include "environment_cfg/environment_export.h" 就能拿到所有
 * 设备句柄, 不需要逐个 extern. 子类完整类型 (struct led_gpio /
 * struct led_pwm / struct led_i2c) 应用层看不到, 只看到基类指针.
 */

#ifndef ENVIRONMENT_CFG_ENVIRONMENT_EXPORT_H_
#define ENVIRONMENT_CFG_ENVIRONMENT_EXPORT_H_

#include "drivers/led/led_base.h"

/* 4 颗 LED, 演示 3 种子类混搭:
 *   led_status   GPIO LED   板上状态指示, 高电平点亮
 *   led_dimmer   PWM  LED   亮度可调, 通道 0
 *   led_panel    I2C  LED   面板 LED 控制器, 0x3C 寄存器 0x00
 *   led_alarm    GPIO LED   告警, 高电平点亮
 */
extern struct led_base *led_status;
extern struct led_base *led_dimmer;
extern struct led_base *led_panel;
extern struct led_base *led_alarm;

#endif /* ENVIRONMENT_CFG_ENVIRONMENT_EXPORT_H_ */
