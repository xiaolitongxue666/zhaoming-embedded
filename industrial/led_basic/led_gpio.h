/**
  ******************************************************************************
  * @file    led_gpio.h
  * @brief   The header file of led_gpio class
  *
  * @details GPIO 拉线类型的 LED 子类. 见第 19 章 § 19.1.
  *          基类 base 字段放第一位, 上转直接 (led_base_t *)me 一行 cast,
  *          回查私有字段也直接 (led_gpio_t *)me, 不需要 container_of
  *          (基类不在第一字段时才需要 container_of, 见第 13 章).
  ******************************************************************************
  */

#ifndef __LED_GPIO_H
#define __LED_GPIO_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "led_base.h"
#include "platform_def.h"

/* Exported types ------------------------------------------------------------*/
typedef struct
{
    led_base_t base;        /* 基类放第一字段, 上转 / 回查都是直接 cast */
    int32_t pin_num;        /* 物理 pin 号, 由 platform_pin_get(pin_name) 解析 */
    bool light_level;       /* 高电平点亮还是低电平点亮 (active high/low) */
} led_gpio_t;

/* Public functions ----------------------------------------------------------*/
platform_err_t led_gpio_init
(led_gpio_t *me, const char *pin_name, bool light_level);

#endif

/******************** END OF FILE ******************END OF FILE****/
