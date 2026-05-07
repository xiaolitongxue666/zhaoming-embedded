/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_i2c.h
  * @brief   The header file of platform i2c bus public functions
  *
  * @details I²C 总线抽象层. 跟 platform_pin / platform_pwm / platform_uart
  *          同一套两层继承结构 (见 platform_device.h):
  *            platform_i2c_bus_device_t (子类)
  *               └── platform_device parent (基类, 第一字段)
  *               └── platform_i2c_bus_device_ops_t *ops (vtable)
  *
  *          上层 (例如温度芯片驱动 / EEPROM 驱动) 调 platform_i2c_master_send /
  *          recv, 看不到底层是 STM32 I2C 外设还是 Linux i2c-dev. 跨芯片移植
  *          时只换 platform_hw_i2c_xxx.c 这一份具体后端.
  ******************************************************************************
  */

#ifndef __PLATFORM_I2C_H
#define __PLATFORM_I2C_H

/* Includes ------------------------------------------------------------------*/
#include "platform_device.h"
#include "cmsis_os.h"

/* Exported macros -----------------------------------------------------------*/
#define PLATFORM_I2C_WR          (0x0000)
#define PLATFORM_I2C_RD          (1u << 0)
#define PLATFORM_I2C_ADDR_10BIT  (1u << 2)  /* this is a ten bit chip address */

/* Exported types ------------------------------------------------------------*/
typedef struct platform_i2c_msg
{
    uint16_t addr;
    uint16_t flags;
    uint16_t len;
    uint8_t  *buf;
} platform_i2c_msg_t;

struct platform_i2c_bus_device;

typedef struct platform_i2c_bus_device_ops
{
    uint32_t (*master_xfer)(struct platform_i2c_bus_device *bus,
                            platform_i2c_msg_t msgs[],
                            uint32_t num);
    uint32_t (*slave_xfer)(struct platform_i2c_bus_device *bus,
                           platform_i2c_msg_t msgs[],
                           uint32_t num);
    platform_err_t (*i2c_bus_control)(struct platform_i2c_bus_device *bus,
                                      uint32_t,
                                      uint32_t);
} platform_i2c_bus_device_ops_t;


typedef struct platform_i2c_bus_device
{
    struct platform_device parent;
    const platform_i2c_bus_device_ops_t *ops;
    osMutexId_t lock;
    void *priv;
} platform_i2c_bus_device_t;

typedef struct platform_i2c_client
{
    struct platform_device               parent;
    platform_i2c_bus_device_t       *bus;
    uint16_t                    client_addr;
} platform_i2c_client_t;

/* Exported functions --------------------------------------------------------*/
platform_err_t
platform_i2c_bus_device_register(platform_i2c_bus_device_t *bus,
                                 const char *bus_name);

platform_i2c_bus_device_t *
platform_i2c_bus_device_find(const char *bus_name);

uint32_t
platform_i2c_transfer(platform_i2c_bus_device_t *bus, platform_i2c_msg_t msgs[],
                      uint32_t num);

platform_err_t
platform_i2c_control(platform_i2c_bus_device_t *bus, uint32_t cmd, uint32_t arg);

platform_err_t
platform_i2c_master_send(platform_i2c_bus_device_t *bus, uint16_t addr,
                         uint16_t flags, const uint8_t *buf, uint32_t count);

platform_err_t
platform_i2c_master_recv(platform_i2c_bus_device_t *bus, uint16_t addr,
                         uint16_t flags, uint8_t *buf, uint32_t count);

PLATFORM_INLINE platform_err_t
platform_i2c_bus_lock(platform_i2c_bus_device_t *bus, uint32_t timeout)
{
    platform_err_t ret = PLATFORM_EOK;
    if (osMutexAcquire(bus->lock, timeout) != osOK) ret = PLATFORM_ERROR;
    return ret;
}

PLATFORM_INLINE platform_err_t
platform_i2c_bus_unlock(platform_i2c_bus_device_t *bus)
{
    platform_err_t ret = PLATFORM_EOK;
    if (osMutexRelease(bus->lock) != osOK) ret = PLATFORM_ERROR;
    return ret;
}

#endif

/******************** END OF FILE ******************END OF FILE****/
