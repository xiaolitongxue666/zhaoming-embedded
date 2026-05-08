/* SPDX-License-Identifier: MIT */
/**
 * @file  sensors.h
 * @brief sensor 模块对外暴露的全局句柄 - 应用层零硬件字样
 *
 * @details
 * 跟 leds.h 同款风格, 不同模块各自一份: leds.h 暴露 LED 句柄,
 * sensors.h 暴露 sensor 句柄. 真实工程一块板上不止 LED 一个外设, 每个
 * 外设各自一对 xxx.h + xxx_board_init.c, 谁的硬件参数谁负责.
 *
 * 本章只暴露一颗温度传感器, 演示"接口风格"(三个 ops 全必填). 子类
 * 实现里 read 返回硬编码值, 重点在父类统一接口的全 assert 行为.
 */

#ifndef SENSORS_H
#define SENSORS_H

#include "sensor_base.h"

extern struct sensor_base *g_temp_sensor;

int sensor_board_init(void);

#endif /* SENSORS_H */
