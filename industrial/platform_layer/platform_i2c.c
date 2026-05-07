/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_i2c.c
  * @brief   I2C 总线抽象层实现 —— bus 注册 / 查找 / master 收发 / 控制接口.
  *
  * @details 见 platform_i2c.h. 本文件把工业项目里两份源文件合并到一起:
  *            (1) i2c_core: bus 注册 / find / transfer / control / master_send /
  *                master_recv —— 这些是上层驱动 (例如 EEPROM / 传感器
  *                client) 直接调的对外 API;
  *            (2) i2c_dev : 把 i2c_bus_device 封装成通用 platform_device,
  *                让 platform_device_read/write/control 也能驱动它. 这是
  *                第 15 章 / 第 20 章里讲的"两层继承"在 I2C 总线上的落地.
  *
  *          线程安全靠 bus->lock (osMutex) 兜底, master_xfer 全程持锁.
  *          ops->master_xfer / ops->slave_xfer / ops->i2c_bus_control 由具体
  *          MCU 后端 (platform_hw_i2c_<chip>.c) 填; 没有的 ops 直接报错返回.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "platform_device.h"
#include "platform_assert.h"
#include "platform_i2c.h"
#include "cmsis_os.h"
#define LOG_TAG "i2c"
#include "elog.h"

/* I2C dev 控制命令 ID. 走 platform_device_control 时通过 cmd 区分.
 * 留在本文件里以保持上层接口对外只暴露一个 platform_i2c.h. */
#define PLATFORM_I2C_DEV_CTRL_RW           (0x21)
#define PLATFORM_I2C_DEV_CTRL_CLK          (0x22)

/* control 命令对应的私有数据 (PLATFORM_I2C_DEV_CTRL_RW). */
typedef struct platform_i2c_priv_data
{
    platform_i2c_msg_t *msgs;
    uint32_t number;
} platform_i2c_priv_data_t;

/* Private function prototypes -----------------------------------------------*/
static platform_err_t
platform_i2c_bus_device_init(platform_i2c_bus_device_t *bus, const char *name);

static uint32_t
i2c_bus_device_read(platform_device_t dev, uint32_t pos, void *buffer,
                    uint32_t count);

static uint32_t
i2c_bus_device_write(platform_device_t dev, uint32_t pos,
                     const void *buffer, uint32_t count);

static platform_err_t
i2c_bus_device_control(platform_device_t dev, int cmd, void *args);

/* Private variables ---------------------------------------------------------*/
/* I2C bus 作为 platform_device 子类时挂的 ops 表.
 * init / open / close 这里都不需要, 留 NULL; read/write/control 三件事
 * 把 platform_device 通用接口转译成 i2c master 收发. */
const static struct platform_device_ops i2c_ops =
{
    NULL,
    NULL,
    NULL,
    i2c_bus_device_read,
    i2c_bus_device_write,
    i2c_bus_device_control
};

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  注册一条 I2C bus, 同时建好 mutex 锁并挂进 platform_device 表.
  * @param  bus      上层填好 ops 的 platform_i2c_bus_device_t 实例.
  * @param  bus_name 总线名, 后续 platform_i2c_bus_device_find 按名查找.
  * @retval PLATFORM_EOK 成功, 其余见 platform_err_t.
  */
platform_err_t
platform_i2c_bus_device_register(platform_i2c_bus_device_t *bus,
                                 const char *bus_name)
{
    platform_err_t ret = PLATFORM_EOK;
    static const osMutexAttr_t attr =
    {
        "i2c_lock",
        osMutexPrioInherit,
        NULL,
        0U
    };

    bus->lock = osMutexNew(&attr);
    if (NULL == bus->lock)
    {
        ret = PLATFORM_ENOMEM;
        goto exit;
    }

    ret = platform_i2c_bus_device_init(bus, bus_name);

    log_d("I2C bus [%s] registered", bus_name);

exit:
    return ret;
}

/**
  * @brief  按名查找一条已注册的 I2C bus.
  * @param  bus_name 总线名, 与 register 时一致.
  * @retval bus 句柄, 找不到返回 NULL.
  */
platform_i2c_bus_device_t *
platform_i2c_bus_device_find(const char *bus_name)
{
    platform_i2c_bus_device_t *ret;

    platform_i2c_bus_device_t *bus;
    platform_device_t dev = platform_device_find(bus_name);
    if (NULL == dev)
    {
        log_e("I2C bus %s not exist", bus_name);
        ret = NULL;
        goto exit;
    }

    bus = (platform_i2c_bus_device_t *)dev->user_data;

    ret = bus;

exit:
    return ret;
}

/**
  * @brief  在指定 bus 上按 msg 数组发起一次 I2C transfer.
  * @param  bus  目标总线.
  * @param  msgs 多段 I2C msg 数组.
  * @param  num  msg 段数.
  * @retval 成功完成的 msg 段数 (0 表示失败).
  */
uint32_t
platform_i2c_transfer(platform_i2c_bus_device_t *bus, platform_i2c_msg_t msgs[],
                      uint32_t num)
{
    uint32_t ret;

    if (bus->ops->master_xfer)
    {
        osMutexAcquire(bus->lock, osWaitForever);
        ret = bus->ops->master_xfer(bus, msgs, num);
        osMutexRelease(bus->lock);
    }
    else
    {
        log_e("I2C bus operation not supported");
        ret = 0;
    }

    return ret;
}

/**
  * @brief  对 bus 下发一个控制命令 (典型: 设置时钟频率).
  * @param  bus 目标总线.
  * @param  cmd 命令 ID, 由具体后端约定.
  * @param  arg 命令参数.
  * @retval 见 platform_err_t.
  */
platform_err_t
platform_i2c_control(platform_i2c_bus_device_t *bus, uint32_t cmd, uint32_t arg)
{
    platform_err_t ret;

    if (bus->ops->i2c_bus_control)
    {
        ret = bus->ops->i2c_bus_control(bus, cmd, arg);
    }
    else
    {
        log_e("I2C bus operation not supported");
        ret = 0;
    }

    return ret;
}

/**
  * @brief  master 模式向 addr 设备写 count 字节.
  * @param  bus   总线.
  * @param  addr  从机地址.
  * @param  flags 控制位 (10-bit / no-stop 等), 见 platform_i2c.h.
  * @param  buf   待发送数据.
  * @param  count 发送字节数.
  * @retval 见 platform_err_t.
  */
platform_err_t
platform_i2c_master_send(platform_i2c_bus_device_t *bus, uint16_t addr,
                         uint16_t flags, const uint8_t *buf, uint32_t count)
{
    platform_err_t ret = PLATFORM_EOK;
    platform_i2c_msg_t msg;

    msg.addr  = addr;
    msg.flags = flags;
    msg.len   = count;
    msg.buf   = (uint8_t *)buf;

    uint32_t result = platform_i2c_transfer(bus, &msg, 1);
    if (result != 1) ret = PLATFORM_EIO;

    return ret;
}

/**
  * @brief  master 模式从 addr 设备读 count 字节.
  * @param  bus   总线.
  * @param  addr  从机地址.
  * @param  flags 控制位 (PLATFORM_I2C_RD 自动加上, 调用方不必再加).
  * @param  buf   接收缓冲区.
  * @param  count 读取字节数.
  * @retval 见 platform_err_t.
  */
platform_err_t
platform_i2c_master_recv(platform_i2c_bus_device_t *bus, uint16_t addr,
                         uint16_t flags, uint8_t *buf, uint32_t count)
{
    platform_err_t ret = PLATFORM_EOK;
    platform_i2c_msg_t msg;
    platform_assert(bus != NULL);

    msg.addr   = addr;
    msg.flags  = flags | PLATFORM_I2C_RD;
    msg.len    = count;
    msg.buf    = buf;

    uint32_t result = platform_i2c_transfer(bus, &msg, 1);
    if (result != 1) ret = PLATFORM_EIO;

    return ret;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  把 I2C bus 封装成 platform_device 注册进设备表.
  *         platform_i2c_bus_device_register 内部调一次, 上层不直接用.
  */
static platform_err_t
platform_i2c_bus_device_init(platform_i2c_bus_device_t *bus, const char *name)
{
    struct platform_device *device;
    platform_assert(bus != NULL);

    device = &bus->parent;

    device->user_data = bus;

    /* 挂 platform_device 通用 ops, 让 read/write/control 走通. */
    device->ops     = &i2c_ops;

    /* 注册到 platform_device 全局表. */
    platform_device_register(device, name, PLATFORM_DEVICE_FLAG_DEFAULT);

    return PLATFORM_EOK;
}

/**
  * @brief  platform_device.read 转 I2C master_recv.
  *         pos 的低 16 位放从机地址, 高 16 位放 flags. 这样上层只用一个
  *         pos 参数就能携带"从哪个 7-bit 地址读 + 用什么控制位".
  */
static uint32_t
i2c_bus_device_read(platform_device_t dev, uint32_t pos, void *buffer,
                    uint32_t count)
{
    uint32_t ret = 0;
    uint16_t addr;
    uint16_t flags;
    platform_i2c_bus_device_t *bus = (platform_i2c_bus_device_t *)dev->user_data;

    platform_assert(bus != NULL);
    platform_assert(buffer != NULL);

    log_d("I2C bus dev [%s] reading %u bytes.", dev->name, count);

    addr = pos & 0xffff;
    flags = (pos >> 16) & 0xffff;

    platform_err_t result =  platform_i2c_master_recv(bus, addr, flags,
                             (uint8_t *)buffer, count);

    if (PLATFORM_EOK == result) ret = count;

    return ret;
}

/**
  * @brief  platform_device.write 转 I2C master_send. pos 编码同 read.
  */
static uint32_t
i2c_bus_device_write(platform_device_t dev, uint32_t pos,
                     const void *buffer, uint32_t count)
{
    uint32_t ret = 0;
    uint16_t addr;
    uint16_t flags;
    platform_i2c_bus_device_t *bus = (platform_i2c_bus_device_t *)dev->user_data;

    platform_assert(bus != NULL);
    platform_assert(buffer != NULL);

    log_d("I2C bus dev [%s] writing %u bytes.", dev->name, count);

    addr = pos & 0xffff;
    flags = (pos >> 16) & 0xffff;

    platform_err_t result =  platform_i2c_master_send(bus, addr, flags,
                             (const uint8_t *)buffer,
                             count);

    if (PLATFORM_EOK == result) ret = count;

    return ret;
}

/**
  * @brief  platform_device.control 分发: RW 走多段 transfer, CLK 走 bus 控制.
  */
static platform_err_t
i2c_bus_device_control(platform_device_t dev, int cmd, void *args)
{
    platform_err_t ret = PLATFORM_EOK;
    platform_i2c_priv_data_t *priv_data;
    platform_i2c_bus_device_t *bus = (platform_i2c_bus_device_t *)dev->user_data;
    uint32_t bus_clock;

    platform_assert(bus != NULL);

    switch (cmd)
    {
    case PLATFORM_I2C_DEV_CTRL_RW:
    {
        priv_data = (platform_i2c_priv_data_t *)args;
        ret = platform_i2c_transfer(bus, priv_data->msgs, priv_data->number);
        if (ret < 0)
        {
            ret = PLATFORM_EIO;
            goto exit;
        }
        break;
    }

    case PLATFORM_I2C_DEV_CTRL_CLK:
    {
        bus_clock = *(uint32_t *)args;
        ret = platform_i2c_control(bus, cmd, bus_clock);
        if (ret < 0)
        {
            ret = PLATFORM_EIO;
            goto exit;
        }
        break;
    }

    default:
        break;
    }

exit:
    return ret;
}

/******************** END OF FILE ******************END OF FILE****/
