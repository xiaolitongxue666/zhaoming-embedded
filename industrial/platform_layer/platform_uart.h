/**
  ******************************************************************************
  * @file    platform_uart.h
  * @brief   The header file of platform UART - UART base class
  *
  * @details UART 抽象层. 标配六大 ops (open/close/read/write/configure) +
  *          硬件配置参数集 (波特率 / 数据位 / 停止位 / 奇偶校验 / 字节序 /
  *          流控). 跟 platform_pin / platform_i2c / platform_spi 同一套
  *          两层继承结构, 上层 / 应用层只见 platform_uart_dev_t * 句柄,
  *          看不到 USART_TypeDef* 寄存器或 HAL_UART_Transmit 调用.
  *
  *          典型用法: 主控板跟外部 Host 通信走 TCP/IP, 跟内部子控板通信
  *          走串口 (见第 19 章主控案例).
  ******************************************************************************
  */


#ifndef __PLATFORM_UART_H_
#define __PLATFORM_UART_H_

/* Includes ------------------------------------------------------------------*/
#include "platform_device.h"

/* Exported macros -----------------------------------------------------------*/

/* IO control command id */
/* UART electrical configuration */
#define PLATFORM_DEVICE_CTRL_UART_CONFIGURE     (0x01)

/* Platform UART electrical configuration param */
#define BAUD_RATE_2400      (2400)
#define BAUD_RATE_4800      (4800)
#define BAUD_RATE_9600      (9600)
#define BAUD_RATE_19200     (19200)
#define BAUD_RATE_38400     (38400)
#define BAUD_RATE_57600     (57600)
#define BAUD_RATE_115200    (115200)
#define BAUD_RATE_230400    (230400)
#define BAUD_RATE_460800    (460800)
#define BAUD_RATE_921600    (921600)
#define BAUD_RATE_2000000   (2000000)
#define BAUD_RATE_3000000   (3000000)

#define DATA_BITS_5         (5)
#define DATA_BITS_6         (6)
#define DATA_BITS_7         (7)
#define DATA_BITS_8         (8)
#define DATA_BITS_9         (9)

#define STOP_BITS_1         (0)
#define STOP_BITS_2         (1)
#define STOP_BITS_3         (2)
#define STOP_BITS_4         (3)

#define PARITY_NONE         (0)
#define PARITY_ODD          (1)
#define PARITY_EVEN         (2)

#define BIT_ORDER_LSB       (0)
#define BIT_ORDER_MSB       (1)

#define NRZ_NORMAL          (0) /* Non Return to Zero : normal mode */
#define NRZ_INVERTED        (1) /* Non Return to Zero : inverted mode */

#define PLATFORM_SERIAL_RX_DEF_BUFSZ    (256)
#define PLATFORM_SERIAL_TX_DEF_BUFSZ    (1024)

#define PLATFORM_SERIAL_FLOWCONTROL_CTSRTS    (1)
#define PLATFORM_SERIAL_FLOWCONTROL_NONE      (0)

/* Default config for platform_uart_configure_t */
#define PLATFORM_SERIAL_CONFIG_DEFAULT \
{\
    BAUD_RATE_115200,                  /* 115200 bits/s */   \
    DATA_BITS_8,                       /* 8 databits */      \
    STOP_BITS_1,                       /* 1 stopbit */       \
    PARITY_NONE,                       /* No parity  */      \
    BIT_ORDER_LSB,                     /* LSB first sent */  \
    NRZ_NORMAL,                        /* Normal mode */     \
    PLATFORM_SERIAL_FLOWCONTROL_NONE,  /* Off flowcontrol */ \
    0                                                        \
}

/* Exported types ------------------------------------------------------------*/
typedef struct platform_uart_configure
{
    uint32_t baud_rate;

    uint32_t data_bits               :4;
    uint32_t stop_bits               :2;
    uint32_t parity                  :2;
    uint32_t bit_order               :1;
    uint32_t invert                  :1;
    uint32_t flowcontrol             :1;
    uint32_t reserved                :5;
} platform_uart_configure_t;

typedef struct platform_uart_device
{
    struct platform_device parent;
    const struct platform_uart_ops *uart_ops;
    platform_uart_configure_t configure;
} platform_uart_dev_t;

typedef struct platform_uart_ops
{
    platform_err_t (*open)(platform_uart_dev_t *puart_dev);

    platform_err_t (*close)(platform_uart_dev_t *puart_dev);

    uint32_t (*read)
    (platform_uart_dev_t *puart_dev, uint32_t pos, void *pbuf, uint32_t size);

    uint32_t (*write)
    (platform_uart_dev_t *puart_dev, uint32_t pos,
     const void *pbuf, uint32_t size);

    platform_err_t (*configure)
    (platform_uart_dev_t *puart_dev, platform_uart_configure_t *pcfg);
} platform_uart_ops_t;

/* Exported functions --------------------------------------------------------*/
platform_err_t platform_hw_uart_register
(platform_uart_dev_t *puart_dev, const char *name, void *pdata);

#endif /* __PLATFORM_UART_H_ */

/******************** END OF FILE ******************END OF FILE****/
