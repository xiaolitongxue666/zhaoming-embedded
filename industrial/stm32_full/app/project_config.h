/**
  ******************************************************************************
  * @file    project_config.h
  * @brief   Project-wide compile-time switches.
  *
  * 脱敏说明: 这一份是 industrial/stm32_full 教学工程的统一配置头. 工业项目
  * 里这里通常还有 PLATFORM_HEAP_ENABLE / PLATFORM_OS / log level 之类的开关,
  * 此处保留最小集合, 其他按需扩展.
  ******************************************************************************
  */

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

/* RTOS / 裸机切换. 0 = bare-metal, 1 = RTOS (FreeRTOS / CMSIS-RTOS v2 等). */
#define PLATFORM_OS                 0

/* heap 子系统. 0 = 不启用 platform_heap_init. */
#define PLATFORM_HEAP_ENABLE        0

/* assert 行为. 1 = 失败时 while(1) 死循环 + 打印; 0 = 仅打印 */
#define PLATFORM_ASSERT_HALT        1

#endif /* PROJECT_CONFIG_H */

/******************** END OF FILE ********************/
