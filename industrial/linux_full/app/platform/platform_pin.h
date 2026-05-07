/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_pin.h
  * @brief   Header file of PIN device driver framework.
  *
  * 见附录 C § C.4 + 第 15 章 "Platform 抽象到底" + 第 16 章 "Linux 不难".
  *
  * 字符串名 -> pin_num 的工业级 GPIO 抽象层. 调用方写 "PA.5" / "PD.12" /
  * "PI.14" / "GPIO17" 这种字面字符串, 看不到底层 GPIO_TypeDef* 寄存器或
  * BCM 编号或 chip line.
  *
  * 这是 RT-Thread / Linux 内核 gpiod 同款做法. 跨芯片移植时只换 board 层
  * pin_libgpiod.c / pin_sysfs.c 子类, 上层 driver / 应用层一字不动.
  ******************************************************************************
  */

#ifndef __PLATFORM_PIN_H
#define __PLATFORM_PIN_H

#include <stdint.h>
#include "platform/platform_def.h"

/* PIN 电平 */
#define PIN_LOW                       0x00
#define PIN_HIGH                      0x01

/* PIN 工作模式 */
#define PIN_MODE_OUTPUT               0x00
#define PIN_MODE_INPUT                0x01
#define PIN_MODE_INPUT_PULLUP         0x02
#define PIN_MODE_INPUT_PULLDOWN       0x03
#define PIN_MODE_OUTPUT_OD            0x04

/* PIN 中断模式 */
#define PIN_IRQ_MODE_RISING           0x00
#define PIN_IRQ_MODE_FALLING          0x01
#define PIN_IRQ_MODE_RISING_FALLING   0x02
#define PIN_IRQ_MODE_HIGH_LEVEL       0x03
#define PIN_IRQ_MODE_LOW_LEVEL        0x04

#define PIN_IRQ_DISABLE               0x00
#define PIN_IRQ_ENABLE                0x01

#define PIN_IRQ_PIN_NONE              -1

/* ------ ops 表抽象 (子类填写) ------------------------------------------- */
typedef struct
{
    void (*pin_mode)(int32_t pin, int32_t mode);
    void (*pin_write)(int32_t pin, int32_t value);
    int32_t (*pin_read)(int32_t pin);
    platform_err_t (*pin_attach_irq)(int32_t pin, uint32_t mode,
                                     void (*hdr)(void *args), void *args);
    platform_err_t (*pin_detach_irq)(int32_t pin);
    platform_err_t (*pin_irq_enable)(int32_t pin, uint32_t enabled);
    int32_t (*pin_get)(const char *name);
} platform_pin_ops_t;

/* ------ 注册接口 (子类用) ----------------------------------------------- */
platform_err_t platform_pin_register(const platform_pin_ops_t *ops);

/* ------ 公共 API (上层调) ----------------------------------------------- */
void platform_pin_mode(int32_t pin, int32_t mode);
void platform_pin_write(int32_t pin, int32_t value);
int32_t platform_pin_read(int32_t pin);

platform_err_t platform_pin_attach_irq(int32_t pin, uint32_t mode,
                                       void (*hdr)(void *args), void *args);
platform_err_t platform_pin_detach_irq(int32_t pin);
platform_err_t platform_pin_irq_enable(int32_t pin, uint32_t enabled);

/* "PA.5" / "PD.12" / "PI.14" -> pin_num */
int32_t platform_pin_get(const char *name);

#endif /* __PLATFORM_PIN_H */

/******************** END OF FILE ********************/
