/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_spi.h
  * @brief   The header file of spi bus and spi dev functions
  *
  * @details SPI 总线 + SPI 设备双层抽象. 一条 SPI bus 上可以挂多个 device
  *          (用片选 CS 区分), 这种 bus/device 双层结构跟 Linux 内核 spi_master /
  *          spi_device 一一对应:
  *            platform_spi_bus_t      (一条物理 SPI 总线 + ops 表)
  *            platform_spi_device_t   (挂在 bus 上的具体外设, 持自己的 config)
  *
  *          上层驱动 (W25Q Flash / SD 卡 / NRF24L01...) 只调 platform_spi_send /
  *          recv / transfer / sendrecv8/16, 不直接碰 SPI 寄存器或 HAL.
  ******************************************************************************
  */

#ifndef __PLATFORM_SPI_H
#define __PLATFORM_SPI_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "platform_device.h"
#include "cmsis_os.h"

/* Exported macros -----------------------------------------------------------*/

/**
 * At CPOL=0 the base value of the clock is zero
 *  - For CPHA=0, data are captured on the clock's rising edge
 * (low->high transition)
 *    and data are propagated on a falling edge (high->low clock transition).
 *  - For CPHA=1, data are captured on the clock's falling edge and data are
 *    propagated on a rising edge.
 * At CPOL=1 the base value of the clock is one (inversion of CPOL=0)
 *  - For CPHA=0, data are captured on clock's falling edge and data are
 * propagated on a rising edge.
 *  - For CPHA=1, data are captured on clock's rising edge and data are
 * propagated on a falling edge.
 */
#define PLATFORM_SPI_CPHA     (1<<0)    /* bit[0]:CPHA, clock phase */
#define PLATFORM_SPI_CPOL     (1<<1)    /* bit[1]:CPOL, clock polarity */

#define PLATFORM_SPI_LSB      (0<<2)    /* bit[2]: 0-LSB */
#define PLATFORM_SPI_MSB      (1<<2)    /* bit[2]: 1-MSB */

#define PLATFORM_SPI_MASTER   (0<<3)    /* SPI master device */
#define PLATFORM_SPI_SLAVE    (1<<3)    /* SPI slave device */

#define PLATFORM_SPI_CS_HIGH  (1<<4)    /* Chipselect active high */
#define PLATFORM_SPI_NO_CS    (1<<5)    /* No chipselect */
#define PLATFORM_SPI_3WIRE    (1<<6)    /* SI/SO pin shared */
#define PLATFORM_SPI_READY    (1<<7)    /* Slave pulls low to pause */

#define PLATFORM_SPI_MODE_MASK    \
(PLATFORM_SPI_CPHA | PLATFORM_SPI_CPOL | PLATFORM_SPI_MSB | PLATFORM_SPI_SLAVE \
| PLATFORM_SPI_CS_HIGH | PLATFORM_SPI_NO_CS | PLATFORM_SPI_3WIRE \
| PLATFORM_SPI_READY)

/* CPOL = 0, CPHA = 0 */
#define PLATFORM_SPI_MODE_0       (0 | 0)

/* CPOL = 0, CPHA = 1 */
#define PLATFORM_SPI_MODE_1       (0 | PLATFORM_SPI_CPHA)

/* CPOL = 1, CPHA = 0 */
#define PLATFORM_SPI_MODE_2       (PLATFORM_SPI_CPOL | 0)

/* CPOL = 1, CPHA = 1 */
#define PLATFORM_SPI_MODE_3       (PLATFORM_SPI_CPOL | PLATFORM_SPI_CPHA)

#define PLATFORM_SPI_BUS_MODE_SPI         (1<<0)

/* Exported types ------------------------------------------------------------*/

/**
 * SPI message structure
 */
typedef struct platform_spi_message
{
    const void *send_buf;
    void *recv_buf;
    uint32_t length;
    struct platform_spi_message *next;

    uint32_t cs_take    : 1;
    uint32_t cs_release : 1;
} platform_spi_message_t;

/**
 * SPI configuration structure
 */
typedef struct platform_spi_configuration
{
    uint8_t mode;
    uint8_t data_width;
    uint16_t reserved;

    uint32_t max_hz;
} platform_spi_configuration_t;

struct platform_spi_ops;
typedef struct platform_spi_bus
{
    struct platform_device parent;
    uint8_t mode;
    const struct platform_spi_ops *spi_ops;

    osMutexId_t lock;
    struct platform_spi_device *owner;
} platform_spi_bus_t;

/**
 * SPI operators
 */
typedef struct platform_spi_ops
{
    platform_err_t (*configure)(struct platform_spi_device *device,
                                platform_spi_configuration_t *configuration);

    platform_err_t (*xfer)(struct platform_spi_device *device,
                           platform_spi_message_t *message);
} platform_spi_ops_t;

/**
 * SPI Virtual BUS, one device must connected to a virtual BUS
 */
typedef struct platform_spi_device
{
    struct platform_device parent;
    platform_spi_bus_t *bus;

    platform_spi_configuration_t config;
    void   *user_data;
} platform_spi_device_t;

#define SPI_DEVICE(dev) ((platform_spi_device_t *)(dev))

/* Exported functions --------------------------------------------------------*/
platform_err_t platform_spi_bus_register(platform_spi_bus_t *bus,
        const char              *name,
        const struct platform_spi_ops *ops);

platform_err_t platform_spi_bus_attach_device
(platform_spi_device_t *device, const char *name,
 const char *bus_name, void *user_data);

platform_err_t platform_spi_take_bus(platform_spi_device_t *device);

platform_err_t platform_spi_release_bus
(platform_spi_device_t *device);

platform_err_t platform_spi_take(platform_spi_device_t *device);

platform_err_t platform_spi_release(platform_spi_device_t *device);

/* Before doing anything with the SPI device, you first need to call
 *this function to configure the bus */
platform_err_t platform_spi_configure
(platform_spi_device_t *device, platform_spi_configuration_t *cfg);

platform_err_t platform_spi_send_then_recv
(platform_spi_device_t *device, const void *send_buf, uint32_t send_length,
 void *recv_buf, uint32_t recv_length);

platform_err_t platform_spi_send_then_send
(platform_spi_device_t *device, const void *send_buf1, uint32_t send_length1,
 const void *send_buf2, uint32_t send_length2);

platform_err_t platform_spi_transfer(platform_spi_device_t *device,
                                     const void           *send_buf,
                                     void                 *recv_buf,
                                     uint32_t             length);


platform_err_t platform_spi_transfer_message
(platform_spi_device_t  *device, platform_spi_message_t *message);

platform_err_t platform_spi_recv(platform_spi_device_t *device,
                                 void *recv_buf,
                                 uint32_t length);


platform_err_t platform_spi_send(platform_spi_device_t *device,
                                 const void *send_buf,
                                 uint32_t length);

uint8_t platform_spi_sendrecv8(platform_spi_device_t *device,
                               uint8_t data);

uint16_t platform_spi_sendrecv16(platform_spi_device_t *device,
                                 uint16_t data);


void platform_spi_message_append(platform_spi_message_t *list,
                                 platform_spi_message_t *message);

#endif /* __PLATFORM_SPI_H */

/******************** END OF FILE ******************END OF FILE****/
