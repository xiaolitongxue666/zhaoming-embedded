/**
  ******************************************************************************
  * @file    led_base.h
  * @brief   The header file of common led interface,
  *          base class for all led classes.
  *
  * @details 见第 19 章 § 19.1 "LED 驱动: 最小继承范例". 这一份是工业项目里
  *          LED 父类层的最小骨架: 一颗 ops 表 (vptr) + 两个纯虚函数 (led_on /
  *          led_off). 应用层只见 led_base_t * 句柄, 永远不知道下层是 GPIO
  *          拉线还是 PWM 调亮度还是 I²C 寄存器写, 切实现把 led_xxx_init
  *          替换即可, 应用层一字不动.
  ******************************************************************************
  */

#ifndef __LED_BASE_H
#define __LED_BASE_H

struct led_base_ops;

/* Exported types ------------------------------------------------------------*/
/**
  * @brief 父类对象. 只放一个 ops 指针, 任何具体子类 (led_gpio_t / led_pwm_t /
  *        led_i2c_t) 把它放在第一字段做向上转型 (见第 12 章 "向上转型").
  */
typedef struct
{
	struct led_base_ops *ops;	/* vptr: 指向子类填充的 ops 表 */
}led_base_t;

/**
  * @brief 父类的虚函数表 (vtable). 子类实例化时填这两根纯虚函数指针,
  *        见第 14 章 "纯虚与抽象类". 不填就是空指针解引用直接死机.
  */
typedef struct led_base_ops
{
    /* pure virtual function: 子类必须实现 */
    void (*led_on)(led_base_t *me);
    void (*led_off)(led_base_t *me);
}led_base_ops_t;

/* Public functions ----------------------------------------------------------*/
/* 父类对外接口. 实现里只做 ops dispatch, 见 led_base.c. */
void led_on(led_base_t *me);
void led_off(led_base_t *me);

#endif

/******************** END OF FILE ******************END OF FILE****/
