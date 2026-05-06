/**
  ******************************************************************************
  * @file    led_base.h
  * @brief   The header file of common LED interface, base class for all LED
  *          subclasses (led_gpio, led_pwm, led_i2c, ...).
  *
  * 见附录 C § C.3 + 第 11 章 "多态完整图景" + 第 14 章 "纯虚与抽象类".
  * 这一份跟 stm32_full 字节级一致, 演示"换硬件不改应用"在 driver 层
  * 0 改动的事实.
  *
  * 应用层只见 led_base_t * 句柄, 永远不知道是 GPIO LED 还是 PWM LED.
  * 切换实现把 led_xxx_init 替换即可, 应用层一字不动.
  ******************************************************************************
  */

#ifndef __LED_BASE_H
#define __LED_BASE_H

struct led_base_ops;

typedef struct
{
    struct led_base_ops *ops;
} led_base_t;

typedef struct led_base_ops
{
    /* pure virtual function: 子类必须填写 */
    void (*led_on)(led_base_t *me);
    void (*led_off)(led_base_t *me);
} led_base_ops_t;

void led_on(led_base_t *me);
void led_off(led_base_t *me);

#endif /* __LED_BASE_H */

/******************** END OF FILE ********************/
