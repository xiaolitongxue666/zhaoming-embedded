/* SPDX-License-Identifier: MIT */
/*
 * project_config.h - Project-wide compile-time switches.
 *
 * 这一份是 industrial/linux_full 教学工程的统一配置头. Linux 用户态比裸机
 * MCU 简单得多, 配置项也少: 这里只保留 assert 行为开关. 工业项目里通常
 * 还会有 log level / 日志路径 / 看门狗超时 之类, 此处保留最小集合.
 */

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

/* assert 行为. 1 = 失败时打印 + abort(); 0 = 仅打印继续跑. */
#define LED_ASSERT_HALT             1

#endif /* PROJECT_CONFIG_H */
