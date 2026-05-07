/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_config.h
  * @brief   平台编译期配置开关。默认走裸机 (PLATFORM_OS == 0)，工业项目
  *          可在自己的工程里覆盖这里的宏定义来切到 RTOS 模式。
  ******************************************************************************
  */
#ifndef PLATFORM_API_PLATFORM_CONFIG_H_
#define PLATFORM_API_PLATFORM_CONFIG_H_

/* OS 模式开关：
 *   0 = 裸机模式（platform_sys_xxx 全部退化为 nop）
 *   1 = RTOS 模式（platform_sys_arch.c 提供具体 OS API 实现）
 */
#ifndef PLATFORM_OS
#define PLATFORM_OS                 (0)
#endif

/* RTOS 模式下指向具体 OS port 的 arch 头文件路径。 */
#ifndef PLATFORM_SYS_ARCH_H
#define PLATFORM_SYS_ARCH_H         "platform_sys_arch.h"
#endif

#endif /* PLATFORM_API_PLATFORM_CONFIG_H_ */
