/**
  ******************************************************************************
  * @file    led_cfg.c
  * @brief   LED instance configuration + initcall registration.
  *
  * 见附录 C § C.3 + 第 17 章 "initcall".
  *
  * 配置层负责装配 LED 实例 (静态分配 + 调子类 init), 导出 led_base_t *
  * 句柄给应用层. 启动期通过 INIT_ENV_EXPORT 自动调用 (Linux 用户态下
  * 退化为 GCC __attribute__((constructor(105))), 在 main 之前自动跑).
  *
  * 树莓派 4B 上 4 颗 LED 的 BCM 引脚 (用杜邦线连 GPIO 到 LED + 220Ω 限流):
  *   GPIO17 = green, GPIO27 = orange, GPIO22 = red, GPIO23 = blue
  *
  * 板子换了改这一份的 4 个 pin 名字符串就行, 上层 driver / 应用层一字不动.
  ******************************************************************************
  */

#include <stddef.h>
#include <stdio.h>

#include "platform/platform_module_export.h"
#include "led/led_gpio.h"

led_base_t *led_green   = NULL;
led_base_t *led_orange  = NULL;
led_base_t *led_red     = NULL;
led_base_t *led_blue    = NULL;

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

    ret = led_gpio_init(&_led_green_inst,  "GPIO17", true);
    if (PLATFORM_EOK != ret) {
        fprintf(stderr, "[led_cfg] green init failed: %d\n", (int)ret);
        goto exit;
    }
    led_green = (led_base_t *)&_led_green_inst;

    ret = led_gpio_init(&_led_orange_inst, "GPIO27", true);
    if (PLATFORM_EOK != ret) {
        fprintf(stderr, "[led_cfg] orange init failed: %d\n", (int)ret);
        goto exit;
    }
    led_orange = (led_base_t *)&_led_orange_inst;

    ret = led_gpio_init(&_led_red_inst,    "GPIO22", true);
    if (PLATFORM_EOK != ret) {
        fprintf(stderr, "[led_cfg] red init failed: %d\n", (int)ret);
        goto exit;
    }
    led_red = (led_base_t *)&_led_red_inst;

    ret = led_gpio_init(&_led_blue_inst,   "GPIO23", true);
    if (PLATFORM_EOK != ret) {
        fprintf(stderr, "[led_cfg] blue init failed: %d\n", (int)ret);
        goto exit;
    }
    led_blue = (led_base_t *)&_led_blue_inst;

exit:
    return;
}
INIT_ENV_EXPORT(_led_cfg);

/******************** END OF FILE ********************/
