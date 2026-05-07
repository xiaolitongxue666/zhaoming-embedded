/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    led_cfg.c
  * @brief   LED instance configuration + initcall registration.
  *
  * 见附录 B § B.4 + 第 17 章 "initcall" + 第 19 章 § 19.3.
  *
  * 配置层负责装配 LED 实例 (静态分配 + 调子类 init), 导出 led_base_t *
  * 句柄给应用层. 启动期通过 INIT_ENV_EXPORT 自动调用 (5 级 ENV, 排在
  * BOARD/PREV/DEVICE/COMPONENT 之后, APP 之前).
  *
  * STM32F407 Discovery 板上 4 颗 LED:
  *   PD.12 = green, PD.13 = orange, PD.14 = red, PD.15 = blue
  *
  * 板子换了改这一份的 pin 名字符串就行, 上层 driver / 应用层一字不动.
  ******************************************************************************
  */

#include <stddef.h>
#include <stdio.h>

#include "platform/platform_module_export.h"
#include "led/led_gpio.h"

/* ------ 应用层句柄 (extern 声明在哪儿? 应用层自己 extern 一下即可) ----- */
led_base_t *led_green   = NULL;
led_base_t *led_orange  = NULL;
led_base_t *led_red     = NULL;
led_base_t *led_blue    = NULL;

/* ------ 静态实例池 ------------------------------------------------------ */
static led_gpio_t _led_green_inst;
static led_gpio_t _led_orange_inst;
static led_gpio_t _led_red_inst;
static led_gpio_t _led_blue_inst;

/**
  * @brief  LED 配置函数, 启动期由 INIT_ENV_EXPORT 自动调用.
  */
static void _led_cfg(void)
{
    platform_err_t ret;

    ret = led_gpio_init(&_led_green_inst,  "PD.12", true);
    if (PLATFORM_EOK != ret)
    {
        printf("[led_cfg] green init failed: %d\n", (int)ret);
        goto exit;
    }
    led_green = (led_base_t *)&_led_green_inst;

    ret = led_gpio_init(&_led_orange_inst, "PD.13", true);
    if (PLATFORM_EOK != ret)
    {
        printf("[led_cfg] orange init failed: %d\n", (int)ret);
        goto exit;
    }
    led_orange = (led_base_t *)&_led_orange_inst;

    ret = led_gpio_init(&_led_red_inst,    "PD.14", true);
    if (PLATFORM_EOK != ret)
    {
        printf("[led_cfg] red init failed: %d\n", (int)ret);
        goto exit;
    }
    led_red = (led_base_t *)&_led_red_inst;

    ret = led_gpio_init(&_led_blue_inst,   "PD.15", true);
    if (PLATFORM_EOK != ret)
    {
        printf("[led_cfg] blue init failed: %d\n", (int)ret);
        goto exit;
    }
    led_blue = (led_base_t *)&_led_blue_inst;

exit:
    return;
}
INIT_ENV_EXPORT(_led_cfg);

/******************** END OF FILE ********************/
