/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_spi.c
  * @brief   SPI 总线 / 设备抽象层实现 —— 把 spi_core + spi_dev 合到一起.
  *
  * @details 见 platform_spi.h. SPI 是一对多总线 (一根 bus + 多个 device,
  *          每个 device 一根独立 CS), 所以这里有两层结构:
  *            (1) platform_spi_bus_t    —— 物理 SPI 控制器 (含 mutex / 当前 owner)
  *            (2) platform_spi_device_t —— 挂在 bus 上的 SPI 从设备
  *          两层都是 platform_device 子类, 第一字段 parent 嵌入基类.
  *
  *          owner 切换时自动重 configure (CPOL/CPHA/速率会随设备走);
  *          mutex 保证多个任务并发访问同一根 bus 时 CS 序列不会乱.
  *
  *          ops 由具体后端 (platform_hw_spi_<chip>.c) 填:
  *            - configure(device, cfg)    —— 写 CR1/CR2 寄存器
  *            - xfer     (device, msg)    —— 一次 cs_take + 数据 + cs_release
  *
  *          上层的 platform_spi_send_then_recv / send_then_send / transfer /
  *          transfer_message / sendrecv8 / sendrecv16 都拆成上面两个 op 的组合.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "platform_spi.h"
#include "platform_assert.h"

/* Private function prototypes -----------------------------------------------*/
static platform_err_t platform_spi_bus_device_init
(platform_spi_bus_t *bus, const char *name);

static platform_err_t platform_spidev_device_init
(platform_spi_device_t *dev, const char *name);

static uint32_t _spi_bus_device_read(platform_device_t dev,
                                     uint32_t pos,
                                     void *buffer,
                                     uint32_t size);

static uint32_t _spi_bus_device_write(platform_device_t dev,
                                      uint32_t pos,
                                      const void *buffer,
                                      uint32_t size);

static platform_err_t _spi_bus_device_control(platform_device_t dev,
        int cmd,
        void *args);

static uint32_t _spidev_device_read(platform_device_t dev,
                                    uint32_t pos,
                                    void *buffer,
                                    uint32_t size);

static uint32_t _spidev_device_write(platform_device_t dev,
                                     uint32_t pos,
                                     const void *buffer,
                                     uint32_t size);

static platform_err_t _spidev_device_control(platform_device_t dev,
        int cmd,
        void *args);

/* Private variables ---------------------------------------------------------*/
/* SPI bus 当 platform_device 用时挂的 ops. read/write 直接转给当前 owner. */
const static struct platform_device_ops spi_bus_ops =
{
    NULL,
    NULL,
    NULL,
    _spi_bus_device_read,
    _spi_bus_device_write,
    _spi_bus_device_control
};

/* SPI device 当 platform_device 用时挂的 ops. read/write 走 bus->xfer. */
const static struct platform_device_ops spi_device_ops =
{
    NULL,
    NULL,
    NULL,
    _spidev_device_read,
    _spidev_device_write,
    _spidev_device_control
};

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  注册一根 SPI bus 到 platform_device 表, 并初始化 mutex / owner / mode.
  * @param  bus  上层填好 parent 的 SPI bus 实例.
  * @param  name bus 名.
  * @param  ops  具体后端实现的 configure + xfer.
  * @retval 见 platform_err_t.
  */
platform_err_t platform_spi_bus_register(platform_spi_bus_t *bus,
        const char *name,
        const struct platform_spi_ops *ops)
{
    const osMutexAttr_t attr =
    {
        "spi_bus_mutex",
        osMutexPrioInherit,
        NULL,
        0U
    };

    platform_err_t ret = PLATFORM_EOK;

    ret = platform_spi_bus_device_init(bus, name);
    if (ret != PLATFORM_EOK) goto exit;

    /* 初始化 mutex 锁 */
    bus->lock = osMutexNew(&attr);
    if (NULL == bus->lock)
    {
        ret = PLATFORM_ENOMEM;
        goto exit;
    }
    /* 设置 ops */
    bus->spi_ops = ops;
    /* 当前没有 owner */
    bus->owner = NULL;
    /* 当前默认按 SPI 模式走 (区别于 SPI flash 复用 IP 等场景) */
    bus->mode = PLATFORM_SPI_BUS_MODE_SPI;

exit:
    return ret;
}

/**
  * @brief  把一个 SPI 从设备挂到指定 bus 上, 同时注册成 platform_device.
  * @param  device    SPI 从设备实例.
  * @param  name      设备名.
  * @param  bus_name  目标 bus 名 (必须事先 register 过).
  * @param  user_data 透传给具体后端的私有数据 (例如 CS GPIO 描述符).
  * @retval 见 platform_err_t.
  */
platform_err_t platform_spi_bus_attach_device
(platform_spi_device_t *device, const char *name,
 const char *bus_name, void *user_data)
{
    platform_err_t ret = PLATFORM_EOK;
    platform_device_t bus;

    /* 先按 bus 名取出物理 SPI bus */
    bus = platform_device_find(bus_name);

    if (NULL == bus)
    {
        ret = PLATFORM_ERROR;
        goto exit;
    }

    device->bus = (platform_spi_bus_t *)bus;

    /* 把 SPI device 注册成 platform_device */
    ret = platform_spidev_device_init(device, name);
    if (ret != PLATFORM_EOK) goto exit;

    memset(&device->config, 0, sizeof(device->config));
    device->parent.user_data = user_data;

    ret = PLATFORM_EOK;

exit:
    return ret;
}

/**
  * @brief  按 cfg 配置 SPI 设备 (mode/data_width/max_hz). 仅当当前是 owner 才生效.
  */
platform_err_t platform_spi_configure
(platform_spi_device_t *device, platform_spi_configuration_t *cfg)
{
    platform_err_t ret = PLATFORM_EOK;

    platform_assert(device != NULL);

    /* 把 cfg 落到 device 自己的 config 上, 后续 owner 切换时按这份 reapply */
    device->config.data_width = cfg->data_width;
    device->config.mode       = cfg->mode & PLATFORM_SPI_MODE_MASK ;
    device->config.max_hz     = cfg->max_hz ;

    if (NULL == device->bus)
    {
        ret = PLATFORM_ERROR;
        goto exit;
    }

    osStatus_t result = osMutexAcquire(device->bus->lock, osWaitForever);
    if (result != osOK)
    {
        ret = PLATFORM_ENOSYS;
        goto exit;
    }

    if (device->bus->owner == device)
    {
        ret = device->bus->spi_ops->configure(device, &device->config);
    }

    /* 释放 bus 锁 */
    osMutexRelease(device->bus->lock);

exit:
    return ret;
}

/**
  * @brief  先发 send_buf1, 紧接着发 send_buf2 (中间不释 CS).
  *         典型用法: 先发寄存器地址再发寄存器数据.
  */
platform_err_t platform_spi_send_then_send
(platform_spi_device_t *device, const void *send_buf1, uint32_t send_length1,
 const void *send_buf2, uint32_t send_length2)
{
    platform_err_t ret = PLATFORM_EOK;
    platform_spi_message_t message;

    platform_assert(device != NULL);
    platform_assert(device->bus != NULL);

    osStatus_t result = osMutexAcquire(device->bus->lock, osWaitForever);
    if (result != osOK)
    {
        ret = PLATFORM_ENOSYS;
        goto exit;
    }

    if (device->bus->owner != device)
    {
        /* owner 不是自己, 重新按本 device 的 cfg 配 SPI bus */
        ret = device->bus->spi_ops->configure(device, &device->config);

        if (ret != PLATFORM_EOK) goto exit2;

        /* 把本 device 设成新 owner */
        device->bus->owner = device;
    }

    /* 第一段: 拉 CS, 不释放 */
    message.send_buf   = send_buf1;
    message.recv_buf   = NULL;
    message.length     = send_length1;
    message.cs_take    = 1;
    message.cs_release = 0;
    message.next       = NULL;

    ret = device->bus->spi_ops->xfer(device, &message);
    if (ret != PLATFORM_EOK) goto exit2;

    /* 第二段: 不再拉 CS, 收尾释放 */
    message.send_buf   = send_buf2;
    message.recv_buf   = NULL;
    message.length     = send_length2;
    message.cs_take    = 0;
    message.cs_release = 1;
    message.next       = NULL;

    ret = device->bus->spi_ops->xfer(device, &message);

exit2:
    osMutexRelease(device->bus->lock);
exit:
    return ret;
}

/**
  * @brief  先发 send_buf, 紧接着读 recv_buf (中间不释 CS).
  *         典型用法: 先发寄存器地址, 紧接着读回该寄存器内容.
  */
platform_err_t platform_spi_send_then_recv
(platform_spi_device_t *device, const void *send_buf, uint32_t send_length,
 void *recv_buf, uint32_t recv_length)
{
    platform_err_t ret = PLATFORM_EOK;
    platform_spi_message_t message;

    platform_assert(device != NULL);
    platform_assert(device->bus != NULL);

    osStatus_t result = osMutexAcquire(device->bus->lock, osWaitForever);
    if (result != osOK)
    {
        ret = PLATFORM_ENOSYS;
        goto exit;
    }

    if (device->bus->owner != device)
    {
        /* owner 不是自己, 重新按本 device 的 cfg 配 SPI bus */
        ret = device->bus->spi_ops->configure(device, &device->config);
        if (ret != PLATFORM_EOK) goto exit2;
        /* 把本 device 设成新 owner */
        device->bus->owner = device;
    }

    /* 发送段 */
    message.send_buf   = send_buf;
    message.recv_buf   = NULL;
    message.length     = send_length;
    message.cs_take    = 1;
    message.cs_release = 0;
    message.next       = NULL;

    ret = device->bus->spi_ops->xfer(device, &message);
    if (ret != PLATFORM_EOK) goto exit2;

    /* 接收段 */
    message.send_buf   = NULL;
    message.recv_buf   = recv_buf;
    message.length     = recv_length;
    message.cs_take    = 0;
    message.cs_release = 1;
    message.next       = NULL;

    ret = device->bus->spi_ops->xfer(device, &message);

exit2:
    osMutexRelease(device->bus->lock);
exit:
    return ret;
}

/**
  * @brief  全双工传输: 同一段时间里同时发 send_buf 收 recv_buf, 长度都是 length.
  *         任一缓冲传 NULL 表示该方向不关心.
  */
platform_err_t platform_spi_transfer(platform_spi_device_t *device,
                                     const void           *send_buf,
                                     void                 *recv_buf,
                                     uint32_t             length)
{
    platform_err_t ret = PLATFORM_EOK;
    platform_spi_message_t message;

    platform_assert(device != NULL);
    platform_assert(device->bus != NULL);

    osStatus_t result = osMutexAcquire(device->bus->lock, osWaitForever);
    if (result != osOK)
    {
        ret = PLATFORM_ENOSYS;
        goto exit;
    }

    if (device->bus->owner != device)
    {
        /* owner 不是自己, 按本 device 的 cfg 重配 */
        ret = device->bus->spi_ops->configure(device, &device->config);
        if (ret != PLATFORM_EOK) goto exit2;

        /* 把本 device 设成新 owner */
        device->bus->owner = device;
    }

    /* 单段消息: 拉 CS + 数据 + 释 CS */
    message.send_buf   = send_buf;
    message.recv_buf   = recv_buf;
    message.length     = length;
    message.cs_take    = 1;
    message.cs_release = 1;
    message.next       = NULL;

    /* 发起传输 */
    ret = device->bus->spi_ops->xfer(device, &message);

exit2:
    osMutexRelease(device->bus->lock);
exit:
    return ret;
}

/**
  * @brief  按链表逐段发送 SPI 消息 (适合上层一次组多段事务的情况).
  */
platform_err_t platform_spi_transfer_message
(platform_spi_device_t  *device, platform_spi_message_t *message)
{
    platform_err_t ret = PLATFORM_EOK;
    platform_spi_message_t *index;

    platform_assert(device != NULL);

    /* 取链表头 */
    index = message;
    if (NULL == index)
    {
        ret = PLATFORM_EINVAL;
        goto exit;
    }

    osStatus_t result = osMutexAcquire(device->bus->lock, osWaitForever);
    if (result != osOK)
    {
        ret = PLATFORM_ENOSYS;
        goto exit;
    }

    /* 必要时重配 SPI bus */
    if (device->bus->owner != device)
    {
        /* owner 不是自己, 按本 device 的 cfg 重配 */
        ret = device->bus->spi_ops->configure(device, &device->config);

        if (ret != PLATFORM_EOK) goto exit2;

        /* 把本 device 设成新 owner */
        device->bus->owner = device;
    }

    /* 逐段下发 */
    while (index != NULL)
    {
        /* 单段 xfer */
        ret = device->bus->spi_ops->xfer(device, index);
        if (ret != PLATFORM_EOK) break;

        index = index->next;
    }

exit2:
    /* 释放 bus 锁 */
    osMutexRelease(device->bus->lock);
exit:
    return ret;
}

/**
  * @brief  显式抢 bus (持锁), 适合上层要做"锁定一组连续事务"的场景.
  */
platform_err_t platform_spi_take_bus(platform_spi_device_t *device)
{
    platform_err_t ret = PLATFORM_EOK;

    platform_assert(device != NULL);
    platform_assert(device->bus != NULL);

    osStatus_t result = osMutexAcquire(device->bus->lock, osWaitForever);
    if (result != osOK)
    {
        ret = PLATFORM_ENOSYS;
        goto exit;
    }

    /* 必要时切 owner */
    if (device->bus->owner != device)
    {
        ret = device->bus->spi_ops->configure(device, &device->config);
        if (PLATFORM_EOK == ret)
        {
            /* 切 owner 成功 */
            device->bus->owner = device;
        }
        else
        {
            /* 配置失败, 立刻释放, 不能让锁挂着 */
            osMutexRelease(device->bus->lock);
        }
    }
exit:
    return ret;
}

/**
  * @brief  与 platform_spi_take_bus 配对释放 bus 锁.
  */
platform_err_t platform_spi_release_bus(platform_spi_device_t *device)
{
    platform_assert(device != NULL);
    platform_assert(device->bus != NULL);
    platform_assert(device->bus->owner == device);

    /* 释放 bus 锁 */
    osMutexRelease(device->bus->lock);

    return PLATFORM_EOK;
}

/**
  * @brief  仅拉 CS (不传数据). 用于上层手动控时序的特殊场景.
  */
platform_err_t platform_spi_take(platform_spi_device_t *device)
{
    platform_err_t ret;
    platform_spi_message_t message;

    platform_assert(device != NULL);
    platform_assert(device->bus != NULL);

    memset(&message, 0, sizeof(message));
    message.cs_take = 1;

    ret = device->bus->spi_ops->xfer(device, &message);

    return ret;
}

/**
  * @brief  仅释 CS (不传数据). 与 platform_spi_take 配对.
  */
platform_err_t platform_spi_release(platform_spi_device_t *device)
{
    platform_err_t ret;
    platform_spi_message_t message;

    platform_assert(device != NULL);
    platform_assert(device->bus != NULL);

    memset(&message, 0, sizeof(message));
    message.cs_release = 1;

    ret = device->bus->spi_ops->xfer(device, &message);

    return ret;
}

/**
  * @brief  纯接收 (主器件输出 0x00 时钟).
  */
platform_err_t platform_spi_recv(platform_spi_device_t *device,
                                 void *recv_buf,
                                 uint32_t length)
{
    return platform_spi_transfer(device, NULL, recv_buf, length);
}

/**
  * @brief  纯发送 (丢弃读回数据).
  */
platform_err_t platform_spi_send(platform_spi_device_t *device,
                                 const void *send_buf,
                                 uint32_t length)
{
    return platform_spi_transfer(device, send_buf, NULL, length);
}

/**
  * @brief  发 1 字节同时收 1 字节. 寄存器读写小事务最常用.
  */
uint8_t platform_spi_sendrecv8(platform_spi_device_t *device,
                               uint8_t data)
{
    uint8_t value;

    platform_spi_send_then_recv(device, &data, 1, &value, 1);

    return value;
}

/**
  * @brief  发 2 字节同时收 2 字节.
  */
uint16_t platform_spi_sendrecv16(platform_spi_device_t *device,
                                 uint16_t data)
{
    uint16_t value;

    platform_spi_send_then_recv(device, &data, 2, &value, 2);

    return value;
}

/**
  * @brief  把 message 追加到 list 尾部, 用来构造多段链式 SPI 事务.
  */
void platform_spi_message_append(platform_spi_message_t *list,
                                 platform_spi_message_t *message)
{
    platform_assert(list != NULL);

    if (message == NULL)
        return; /* 不追加 */

    while (list->next != NULL)
    {
        list = list->next;
    }

    list->next = message;
    message->next = NULL;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  把 SPI bus 注册进 platform_device 表, 让它可以被 find/read/write 命中.
  */
static platform_err_t platform_spi_bus_device_init
(platform_spi_bus_t *bus, const char *name)
{
    struct platform_device *device;
    platform_assert(bus != NULL);

    device = &bus->parent;

    /* 挂通用 ops */
    device->ops     = &spi_bus_ops;

    /* 注册到设备表 */
    return platform_device_register(device, name, PLATFORM_DEVICE_FLAG_DEFAULT);
}

/**
  * @brief  把 SPI 从设备注册进 platform_device 表.
  */
static platform_err_t platform_spidev_device_init
(platform_spi_device_t *dev, const char *name)
{
    struct platform_device *device;
    platform_assert(dev != NULL);

    device = &(dev->parent);

    device->ops     = &spi_device_ops;

    /* 注册到设备表 */
    return platform_device_register(device, name, PLATFORM_DEVICE_FLAG_DEFAULT);
}

/* SPI bus 当 platform_device 用时的 read: 直接转给当前 owner. */
static uint32_t _spi_bus_device_read(platform_device_t dev,
                                     uint32_t pos,
                                     void *buffer,
                                     uint32_t size)
{
    platform_spi_bus_t *bus;

    bus = (platform_spi_bus_t *)dev;
    platform_assert(bus != NULL);
    platform_assert(bus->owner != NULL);

    return platform_spi_transfer(bus->owner, NULL, buffer, size);
}

/* SPI bus 当 platform_device 用时的 write. */
static uint32_t _spi_bus_device_write(platform_device_t dev,
                                      uint32_t pos,
                                      const void *buffer,
                                      uint32_t size)
{
    platform_spi_bus_t *bus;

    bus = (platform_spi_bus_t *)dev;
    platform_assert(bus != NULL);
    platform_assert(bus->owner != NULL);

    return platform_spi_transfer(bus->owner, buffer, NULL, size);
}

/* SPI bus 控制命令分发占位. 暂未定义具体 cmd, 由后端按需扩展. */
static platform_err_t _spi_bus_device_control(platform_device_t dev,
        int cmd,
        void *args)
{
    /* TODO: 后端按需添加控制命令处理 */
    switch (cmd)
    {
    case 0:
        break;
    case 1:
        break;
    }

    return PLATFORM_EOK;
}

/* SPI device 当 platform_device 用时的 read. */
static uint32_t _spidev_device_read(platform_device_t dev,
                                    uint32_t pos,
                                    void *buffer,
                                    uint32_t size)
{
    platform_spi_device_t *device;

    device = (platform_spi_device_t *)dev;
    platform_assert(device != NULL);
    platform_assert(device->bus != NULL);

    return platform_spi_transfer(device, NULL, buffer, size);
}

/* SPI device 当 platform_device 用时的 write. */
static uint32_t _spidev_device_write(platform_device_t dev,
                                     uint32_t pos,
                                     const void *buffer,
                                     uint32_t size)
{
    platform_spi_device_t *device;

    device = (platform_spi_device_t *)dev;
    platform_assert(device != NULL);
    platform_assert(device->bus != NULL);

    return platform_spi_transfer(device, buffer, NULL, size);
}

/* SPI device 控制命令分发占位. */
static platform_err_t _spidev_device_control(platform_device_t dev,
        int cmd,
        void *args)
{
    switch (cmd)
    {
    case 0:
        break;
    case 1:
        break;
    }

    return PLATFORM_EOK;
}

/******************** END OF FILE ******************END OF FILE****/
