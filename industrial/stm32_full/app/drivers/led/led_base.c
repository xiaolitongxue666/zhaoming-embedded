/**
  ******************************************************************************
  * @file    led_base.c
  * @brief   Common LED interface, base class dispatch.
  *
  * 见附录 B § B.3 + 第 11 章 "多态完整图景".
  *
  * 父类层在调用子类 ops 之前三层 platform_assert 收拢参数校验:
  *   1. me 本身合法
  *   2. me->ops 已填充
  *   3. 子类必填的 ops 函数指针 已填充
  *
  * 三层校验是工业级硬规则, 防示子类忘填纯虚函数 (第 14 章主题)
  * 或实例初始化路径出错使 ops 仅部分填充.
  ******************************************************************************
  */

#include <stddef.h>

#include "led/led_base.h"
#include "platform/platform_assert.h"

/**
  * @brief  Turn on LED via base ops dispatch.
  * @param  me  Base this pointer.
  */
void led_on(led_base_t *me)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->led_on != NULL);

    me->ops->led_on(me);
}

/**
  * @brief  Turn off LED via base ops dispatch.
  * @param  me  Base this pointer.
  */
void led_off(led_base_t *me)
{
    platform_assert(me != NULL);
    platform_assert(me->ops != NULL);
    platform_assert(me->ops->led_off != NULL);

    me->ops->led_off(me);
}

/******************** END OF FILE ********************/
