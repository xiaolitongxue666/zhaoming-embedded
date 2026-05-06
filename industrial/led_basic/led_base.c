/**
  ******************************************************************************
  * @file    led_base.c
  * @brief   Common led interface, base class for all led classes.
  *
  * @details 见第 19 章 § 19.1. 父类 .c 只做一件事: 把对外 API 转发给子类
  *          ops 表里的对应函数指针. 这就是第 11 章 "多态完整图景" 推出来的
  *          vtable 查表 + 间接跳转, 工业代码里一字不差.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "led_base.h"

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  turn on led
  * @param  me - this pointer
  * @retval none
  */
void led_on(led_base_t *me)
{
    return me->ops->led_on(me);
}

/**
  * @brief  turn off led
  * @param  me - this pointer
  * @retval none
  */
void led_off(led_base_t *me)
{
    return me->ops->led_off(me);
}

/******************** END OF FILE ******************END OF FILE****/
