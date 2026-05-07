/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    led_gpio.h
  * @brief   The header file of led_gpio subclass.
  *
  * 见附录 B § B.3. GPIO 拉线类型的 LED 子类. 基类放第一字段, 上转 / 回查
  * 都是直接 cast (基类不在第一字段时才需要 container_of, 见第 13 章).
  ******************************************************************************
  */

#ifndef __LED_GPIO_H
#define __LED_GPIO_H

#include <stdint.h>
#include <stdbool.h>

#include "led/led_base.h"
#include "platform/platform_def.h"

typedef struct
{
    led_base_t base;        /* 基类放第一字段, 上转直接 cast */
    int32_t    pin_num;
    bool       light_level;
} led_gpio_t;

/**
  * @brief  Constructor.
  * @param  me           This pointer.
  * @param  pin_name     Platform pin name, eg "PA.5", "PD.12".
  * @param  light_level  Output level when LED is on (true = active high).
  * @retval See platform_err_t.
  */
platform_err_t led_gpio_init(
    led_gpio_t *me, const char *pin_name, bool light_level);

#endif /* __LED_GPIO_H */

/******************** END OF FILE ********************/
