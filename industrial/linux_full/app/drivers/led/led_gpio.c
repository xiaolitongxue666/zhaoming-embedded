/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    led_gpio.c
  * @brief   The implementation of led_gpio subclass.
  *
  * 见附录 C § C.3 + 第 15 章 "Platform 抽象到底" + 第 12 章 "向上转型".
  *
  * 子类只调 platform_pin 封装函数, 永远不直接碰 GPIO 寄存器, 也不
  * include platform_pin 内部头文件. 跨芯片移植时这一份代码 0 改动 —
  * 这一份跟 stm32_full 字节级一致, 是"换硬件不改应用"最直接的证据.
  ******************************************************************************
  */

#include <stddef.h>

#include "led/led_gpio.h"
#include "platform/platform_pin.h"

/* ------ ops 子类实现 ---------------------------------------------------- */
static void _led_gpio_on(led_base_t *me);
static void _led_gpio_off(led_base_t *me);

/* 同一个 ops 表给所有 led_gpio 实例共享, const 放 Flash. */
static const led_base_ops_t _ops =
{
    .led_on  = _led_gpio_on,
    .led_off = _led_gpio_off,
};

/**
  * @brief  Constructor.
  * @param  me           This pointer.
  * @param  pin_name     Platform pin name (eg "PA.5", "PD.12").
  * @param  light_level  Output level when LED is on.
  * @retval See platform_err_t.
  */
platform_err_t led_gpio_init(
    led_gpio_t *me, const char *pin_name, bool light_level)
{
    platform_err_t ret = PLATFORM_EOK;
    int32_t pin_num;

    if ((NULL == me) || (NULL == pin_name))
    {
        ret = PLATFORM_EINVAL;
        goto exit;
    }

    pin_num = platform_pin_get(pin_name);
    if (pin_num < 0)
    {
        ret = PLATFORM_EINVAL;
        goto exit;
    }

    platform_pin_mode(pin_num, PIN_MODE_OUTPUT);
    platform_pin_write(pin_num, !light_level);

    me->pin_num     = pin_num;
    me->light_level = light_level;
    me->base.ops    = (led_base_ops_t *)&_ops;

exit:
    return ret;
}

/* ------ private ops --------------------------------------------------- */

/**
  * @brief  Turn on the LED.
  * @param  me  Base this pointer.
  */
static void _led_gpio_on(led_base_t *me)
{
    led_gpio_t *self = (led_gpio_t *)me;
    platform_pin_write(self->pin_num, self->light_level);
}

/**
  * @brief  Turn off the LED.
  * @param  me  Base this pointer.
  */
static void _led_gpio_off(led_base_t *me)
{
    led_gpio_t *self = (led_gpio_t *)me;
    platform_pin_write(self->pin_num, !self->light_level);
}

/******************** END OF FILE ********************/
