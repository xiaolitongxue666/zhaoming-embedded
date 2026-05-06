/**
  ******************************************************************************
  * @file    platform_pin.h
  * @brief   Header file of PIN device driver framework.
  *
  * @details 见第 15 章 "Platform 抽象到底" + 第 20 章 § 20.6. 字符串名 ->
  *          pin_num 的工业级 GPIO 抽象层. 调用方写 "PA.5" / "PD.12" /
  *          "PI.14" 字面字符串, 看不到 GPIO_TypeDef* 寄存器或 port 索引.
  *
  *          这是 RT-Thread / Linux 内核 gpiod 同款做法. 跨芯片移植时
  *          只换 board 层 pin 子类, 上层 driver / 应用层一字不动.
  *
  *          这个版本继承自 platform_device 通用设备基类 (走两层继承结构,
  *          见 platform_device.h), 比附录 B/C 教学版略复杂 — 教学版把
  *          ops 直接塞到一个 static 全局, 这一份保留了工业级"设备注册表"
  *          的通用框架.
  ******************************************************************************
  */

#ifndef __PLATFORM_PIN_H
#define __PLATFORM_PIN_H

/* Includes ------------------------------------------------------------------*/
#include <platform/platform_device.h>

/* Exported defines ----------------------------------------------------------*/
#define PIN_LOW                 0x00
#define PIN_HIGH                0x01

#define PIN_MODE_OUTPUT         0x00
#define PIN_MODE_INPUT          0x01
#define PIN_MODE_INPUT_PULLUP   0x02
#define PIN_MODE_INPUT_PULLDOWN 0x03
#define PIN_MODE_OUTPUT_OD      0x04

#define PIN_IRQ_MODE_RISING             0x00
#define PIN_IRQ_MODE_FALLING            0x01
#define PIN_IRQ_MODE_RISING_FALLING     0x02
#define PIN_IRQ_MODE_HIGH_LEVEL         0x03
#define PIN_IRQ_MODE_LOW_LEVEL          0x04

#define PIN_IRQ_DISABLE                 0x00
#define PIN_IRQ_ENABLE                  0x01

#define PIN_IRQ_PIN_NONE                -1

/* Exported types ------------------------------------------------------------*/

typedef struct
{
    void (*pin_mode)(struct platform_device *device, int32_t pin, int32_t mode);
    void (*pin_write)(struct platform_device *device, int32_t pin,
                      int32_t value);
    int32_t (*pin_read)(struct platform_device *device, int32_t pin);
    platform_err_t (*pin_attach_irq)(struct platform_device *device,
                                     int32_t pin, uint32_t mode,
                                     void (*hdr)(void *args), void *args);
    platform_err_t (*pin_detach_irq)(struct platform_device *device,
                                     int32_t pin);
    platform_err_t (*pin_irq_enable)(struct platform_device *device,
                                     int32_t pin, uint32_t enabled);
    int32_t (*pin_get)(const char *name);
}platform_pin_ops_t;

/* pin device and operations */
typedef struct
{
    struct platform_device parent;
    const platform_pin_ops_t *ops;
}platform_device_pin_t;

typedef struct
{
    uint16_t pin;
    uint16_t mode;
}platform_device_pin_mode_t;

typedef struct
{
    uint16_t pin;
    uint16_t status;
}platform_device_pin_status_t;

typedef struct
{
    int16_t pin;
    uint16_t mode;
    void (*hdr)(void *args);
    void             *args;
}platform_pin_irq_hdr_t;

/* Exported functions --------------------------------------------------------*/
platform_err_t platform_device_pin_register(
    const char *name, const platform_pin_ops_t *ops, void *user_data);

void platform_pin_mode(int32_t pin, int32_t mode);

void platform_pin_write(int32_t pin, int32_t value);

int32_t  platform_pin_read(int32_t pin);

platform_err_t platform_pin_attach_irq(int32_t pin, uint32_t mode,
                                       void (*hdr)(void *args), void  *args);
platform_err_t platform_pin_detach_irq(int32_t pin);

platform_err_t platform_pin_irq_enable(
    int32_t pin, uint32_t enabled);

/* Get pin number by name,such as PA.0,P0.12 */
int32_t platform_pin_get(const char *name);

#endif /* __PLATFORM_PIN_H */

/******************** END OF FILE ******************END OF FILE****/
