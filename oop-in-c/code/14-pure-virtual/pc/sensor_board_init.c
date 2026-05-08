/* SPDX-License-Identifier: MIT */
/**
 * @file  sensor_board_init.c
 * @brief sensor 模块板级配置 - 全工程唯一认识 sensor 硬件的文件
 *
 * @details
 * 跟 led_board_init.c 同款风格. 真实工程一块板上每个外设各自一份
 * xxx_board_init.c, 谁的硬件参数谁负责.
 *
 * 本章只演示一颗温度传感器, 风格是"接口"(三个 ops 全必填). 调
 * sensor_self_test / sensor_calibrate / sensor_read 时父类统一接口
 * 全 assert, 子类 ops 表少一个调试期立刻爆.
 */

#include "sensors.h"
#include "sensor_temp.h"
#include <stdio.h>

static struct temp_sensor s_temp;

struct sensor_base *g_temp_sensor;

int sensor_board_init(void)
{
	int rc = temp_sensor_init(&s_temp, "TEMP");
	if (rc != 0) {
		printf("[sensor_board] temp_sensor_init failed, rc=%d\n", rc);
		return rc;
	}

	g_temp_sensor = &s_temp.base;
	return 0;
}
