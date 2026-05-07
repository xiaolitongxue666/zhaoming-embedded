/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_uart.c
  * @brief   UART 抽象层基类实现 —— platform_device 通用 ops 转 uart_ops 五件套.
  *
  * @details 见 platform_uart.h. UART 是典型的"基类有 5 个虚函数,
  *          子类填谁就动谁"模型, 也是第 20 章里反复出现的样本.
  *
  *          基类 (本文件) 做的事:
  *            (1) platform_hw_uart_register: 把 platform_uart_dev_t 挂成
  *                platform_device, 并填默认 115200/8N1 配置;
  *            (2) 通用 ops (init/open/close/read/write/control) 接住外部
  *                调用, 把 platform_device * 上转回 platform_uart_dev_t *,
  *                再分发给 puart_dev->uart_ops 上的具体后端实现 (open/close/
  *                read/write/configure).
  *
  *          子类 (具体 MCU 后端 platform_hw_uart_<chip>.c) 只需要实现
  *          uart_ops 五件套, 不用关心 ref_count / mutex / 注册表这些事.
  *
  *          control 走的是 PLATFORM_DEVICE_CTRL_UART_CONFIGURE 命令 ID,
  *          带一份 platform_uart_configure_t 进去, 内部宏 TRY_DO_UART_FUNC
  *          兜空指针: 后端没填 configure 时直接返 EOK.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "platform_uart.h"
#include "platform_assert.h"

/* Private function prototypes -----------------------------------------------*/
static platform_err_t platform_uart_init(struct platform_device *pdev);

static platform_err_t platform_uart_open(struct platform_device *pdev);

static platform_err_t platform_uart_close
(struct platform_device *pdev);

static uint32_t platform_uart_read
(struct platform_device *pdev, uint32_t pos, void *pbuf, uint32_t size);

static uint32_t platform_uart_write
(struct platform_device *pdev, uint32_t pos, const void *pbuf, uint32_t size);

static platform_err_t platform_uart_control
(struct platform_device *pdev, int cmd, void *args);

/* Private variables ---------------------------------------------------------*/
/* UART 基类挂的通用 ops. 全员靠上转 + 分发到 uart_ops. */
const static struct platform_device_ops uart_core_ops =
{
    .init = platform_uart_init,
    .open = platform_uart_open,
    .close = platform_uart_close,
    .read = platform_uart_read,
    .write = platform_uart_write,
    .control = platform_uart_control,
};

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  注册一个 UART 设备到 platform_device 表.
  * @param  puart_dev 上层填好 uart_ops 的 UART 设备实例.
  * @param  name      设备名.
  * @param  pdata     透传给后端的私有数据 (例如 USART 句柄).
  * @retval 见 platform_err_t.
  */
platform_err_t platform_hw_uart_register
(platform_uart_dev_t *puart_dev, const char *name, void *pdata)
{
    platform_uart_configure_t cfg = PLATFORM_SERIAL_CONFIG_DEFAULT;

    platform_assert(puart_dev != NULL);

    struct platform_device *pdev = &(puart_dev->parent);

    /* 默认 115200/8N1, 上层可以再 control 覆盖 */
    puart_dev->configure = cfg;

    pdev->rx_indicate = NULL;
    pdev->tx_complete = NULL;

    pdev->ops = &uart_core_ops;

    pdev->user_data = pdata;

    /* 注册成字符设备 (一开一份, 不可被多 open) */
    return platform_device_register(pdev, name,
                                    PLATFORM_DEVICE_FLAG_STANDALONE);
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  init: platform_device_init 路径下被调, 把当前 cfg 应用到硬件.
  */
static platform_err_t platform_uart_init(struct platform_device *pdev)
{
    platform_err_t ret = PLATFORM_EOK;

    platform_assert(pdev != NULL);

    platform_uart_dev_t *puart_dev = (platform_uart_dev_t *)pdev;

    if (puart_dev->uart_ops->configure)
    {
        ret = puart_dev->uart_ops->configure(puart_dev, &(puart_dev->configure));
    }

    return ret;
}

/**
  * @brief  open: 转给后端 uart_ops->open. 空指针视为不需要打开动作.
  */
static platform_err_t platform_uart_open(struct platform_device *pdev)
{
    platform_err_t ret = PLATFORM_EOK;

    platform_assert(pdev != NULL);

    platform_uart_dev_t *puart_dev = (platform_uart_dev_t *)pdev;

    if (puart_dev->uart_ops->open)
    {
        ret = puart_dev->uart_ops->open(puart_dev);
    }

    return ret;
}

/**
  * @brief  close: 转给后端 uart_ops->close.
  */
static platform_err_t platform_uart_close(struct platform_device *pdev)
{
    platform_err_t ret = PLATFORM_EOK;

    platform_assert(pdev != NULL);

    platform_uart_dev_t *puart_dev = (platform_uart_dev_t *)pdev;

    if (puart_dev->uart_ops->close)
    {
        ret = puart_dev->uart_ops->close(puart_dev);
    }

    return ret;
}

/**
  * @brief  read: 转给后端 uart_ops->read. 没填 read 直接返 0.
  */
static uint32_t platform_uart_read
(struct platform_device *pdev, uint32_t pos, void *pbuf, uint32_t size)
{
    uint32_t read_cnt = 0;

    platform_assert(pdev != NULL);

    platform_uart_dev_t *puart_dev = (platform_uart_dev_t *)pdev;

    if (puart_dev->uart_ops->read)
    {
        read_cnt = puart_dev->uart_ops->read(puart_dev, pos, pbuf, size);
    }

    return read_cnt;
}

/**
  * @brief  write: 转给后端 uart_ops->write.
  */
static uint32_t platform_uart_write
(struct platform_device *pdev, uint32_t pos, const void *pbuf, uint32_t size)
{
    uint32_t write_cnt = 0;

    platform_assert(pdev != NULL);

    platform_uart_dev_t *puart_dev = (platform_uart_dev_t *)pdev;

    if (puart_dev->uart_ops->write)
    {
        write_cnt = puart_dev->uart_ops->write(puart_dev, pos, pbuf, size);
    }

    return write_cnt;
}

/**
  * @brief  control: 目前只识 PLATFORM_DEVICE_CTRL_UART_CONFIGURE.
  *         设备已被上层 open (ref_count != 0) 时不允许重配, 返回 EBUSY.
  */
static platform_err_t platform_uart_control
(struct platform_device *pdev, int cmd, void *args)
{
#define TRY_DO_UART_FUNC(platform_uart_dev, func_name, args) \
    platform_uart_dev->uart_ops->func_name \
    ? platform_uart_dev->uart_ops->func_name(platform_uart_dev, args) \
    : PLATFORM_EOK;

    platform_err_t ret = PLATFORM_EINVAL;

    platform_assert(pdev != NULL);

    platform_uart_dev_t *puart_dev = (platform_uart_dev_t *)pdev;

    switch (cmd)
    {
    case PLATFORM_DEVICE_CTRL_UART_CONFIGURE:
    {
        platform_uart_configure_t *pcfg = (platform_uart_configure_t *)args;

        if (pdev->ref_count)
        {
            /* 已被 open, 不允许中途改 baud/data/stop 等参数 */
            ret = PLATFORM_EBUSY;
            break;
        }

        puart_dev->configure = *pcfg;

        ret = TRY_DO_UART_FUNC(puart_dev, configure, pcfg)

        break;
    }

    default:
        break;
    }

    return ret;

#undef TRY_DO_UART_FUNC
}

/******************** END OF FILE ******************END OF FILE****/
