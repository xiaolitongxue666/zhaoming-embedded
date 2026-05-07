/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_device.h
  * @brief   The header file of platform device.
  *          This file defines abstract platform device class and universal
  *          operating interfaces for platform device.
  *
  * @details 见第 15 章 "Platform 抽象" + 第 20 章 "子控案例". 工业项目里
  *          所有设备 (uart / i2c / spi / pin / pwm / ...) 都继承这一份通用基类:
  *
  *            struct platform_device {
  *                const char *name;             // 设备名, 用 platform_device_find 按名查找
  *                ...                           // 引用计数 / open flag / mutex
  *                const struct platform_device_ops *ops;   // 6 个虚函数 (init/open/close/read/write/control)
  *            };
  *
  *          子类 (platform_uart_dev_t / platform_i2c_bus_device_t / ...) 第一字段
  *          parent 嵌入这个基类, 上转 / container_of 回查直接走第 12-13 章那套.
  *
  *          这是 Linux 内核 device 模型 + RT-Thread 设备框架同款架构, 工业代码
  *          里所有外设驱动都按这一套继承, 应用层调 platform_device_read/write
  *          统一接口, 不需要管下面是哪种外设.
  ******************************************************************************
  */

#ifndef PLATFORM_API_PLATFORM_DEVICE_H_
#define PLATFORM_API_PLATFORM_DEVICE_H_

#include "stdint.h"
#include "string.h"
#include "platform_sys.h"

/**
 *	open device flags defitions
 */
#define PLATFORM_DEVICE_OFLAG_CLOSE			(0x000)
#define PLATFORM_DEVICE_OFLAG_OPEN			(0x001)
#define PLATFORM_DEVICE_OFLAG_INITIALIZED	(0x002)

#define PLATFORM_DEVICE_OFLAG_MASK			(0x0f)

/**
 *	device flags defitions
 */
#define PLATFORM_DEVICE_FLAG_DEFAULT		(0x000)
#define PLATFORM_DEVICE_FLAG_STANDALONE		(0x001)

#define PLATFORM_DEVICE_FLAG_MASK			(0x0f)


#define MAX_PLATFORM_DEV_NUM	(20)

struct platform_device_ops;

typedef struct platform_device *platform_device_t;

struct platform_device
{
	const char* name;
	uint32_t dev_id;
	uint8_t ref_count;
	uint16_t flag;
	uint16_t open_flag;
	platform_sys_mutex_t dev_mutex;
	/* device call back */
	platform_err_t (*rx_indicate)(platform_device_t dev, uint32_t size);
	platform_err_t (*tx_complete)(platform_device_t dev, void *buffer);

	/* common device interface */
	const struct platform_device_ops *ops;
	void                     *user_data;                /**< device private data */
};

struct platform_device_ops
{
    /* common device interface */
    platform_err_t  (*init)   (platform_device_t dev);
    platform_err_t  (*open)   (platform_device_t dev);
    platform_err_t  (*close)  (platform_device_t dev);
    uint32_t (*read)   (platform_device_t dev, uint32_t pos, void *buffer, uint32_t size);
    uint32_t (*write)  (platform_device_t dev, uint32_t pos, const void *buffer, uint32_t size);
    platform_err_t  (*control)(platform_device_t dev, int cmd, void *args);
};

/* static function */
extern platform_err_t platform_device_register(platform_device_t dev, const char *name, uint16_t flags);
extern platform_err_t platform_device_unregister(platform_device_t dev);
extern platform_device_t platform_device_find(const char *name);

/* virtual function  - interface */
extern platform_err_t platform_device_init(platform_device_t dev);
extern platform_err_t platform_device_open(platform_device_t dev);
extern platform_err_t platform_device_close(platform_device_t dev);
extern uint32_t platform_device_read(platform_device_t dev,
                         uint32_t    pos,
                         void       *buffer,
                         uint32_t   size);
extern uint32_t platform_device_write(platform_device_t dev,
                          uint32_t    pos,
                          const void *buffer,
                          uint32_t   size);
extern platform_err_t platform_device_control(platform_device_t dev, int cmd, void *arg);

/* public function */
extern platform_err_t
platform_device_set_rx_indicate(platform_device_t dev,
                          platform_err_t (*rx_ind)(platform_device_t dev, uint32_t size));
extern platform_err_t
platform_device_set_tx_complete(platform_device_t dev,
                          platform_err_t (*tx_done)(platform_device_t dev, void *buffer));


#endif /* PLATFORM_API_PLATFORM_DEVICE_H_ */

/******************** END OF FILE ******************END OF FILE****/

