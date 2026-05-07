/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_assert.c
  * @brief   Default platform_assert_handler implementation.
  *
  * 真机或 OS 环境下可重写为更复杂的逻辑 (写日志 / 触发 watchdog 复位 /
  * 推送告警). 此处给出最小骨架: printf 现场 + 死循环 (PLATFORM_ASSERT_HALT
  * 控制).
  ******************************************************************************
  */

#include "platform/platform_assert.h"
#include "project_config.h"

#include <stdio.h>

/**
  * @brief  Default assert handler.
  * @param  ex_string  Failed expression as string.
  * @param  func       __FUNCTION__ at the assert site.
  * @param  line       __LINE__ at the assert site.
  */
void platform_assert_handler(
    const char *ex_string, const char *func, int line)
{
    printf("[ASSERT] (%s) failed at %s():%d\n",
           ex_string, func, line);

#if (PLATFORM_ASSERT_HALT)
    while (1)
    {
        ;   /* spin forever; 真机用 NVIC_SystemReset() 或 watchdog 强复位 */
    }
#endif
}

/******************** END OF FILE ********************/
