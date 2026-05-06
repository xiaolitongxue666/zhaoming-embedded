/**
  ******************************************************************************
  * @file    platform_pin.c
  * @brief   PIN device driver framework.
  *
  * @details 见第 15 章 "Platform 抽象到底" + 第 20 章 § 20.6. 平台层
  *          "对外封装、对内 ops" 的标准结构: 维护一个 static pin 设备实例
  *          _hw_pin, 上层调 platform_pin_xxx 时通过 ops 表分发到底层 board
  *          子类 (STM32 HAL / libgpiod / sysfs / 模拟). 子类启动期调
  *          platform_device_pin_register(name, &ops, ...) 把自己挂上来.
  *
  *          注意: read/write/control 这三个分发是把 pin 接成"通用 character
  *          device" 接口, 让上层可以用统一的 platform_device_read/write/control
  *          访问任何设备 (Linux 内核 cdev 同款做法).
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "platform_pin.h"
#include "platform_assert.h"

/* Private function prototypes -----------------------------------------------*/
static uint32_t _pin_read(
    struct platform_device * dev, uint32_t pos, void *buffer, uint32_t size);

static uint32_t _pin_write(
    struct platform_device * dev, uint32_t pos,
    const void *buffer, uint32_t size);

static platform_err_t _pin_control(
    struct platform_device * dev, int cmd, void *args);

/* Private variables ---------------------------------------------------------*/
static platform_device_pin_t _hw_pin;

const static struct platform_device_ops pin_ops =
{
    NULL,
    NULL,
    NULL,
    _pin_read,
    _pin_write,
    _pin_control
};

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Platform pin device register
  * @param  name      device name
  * @param  user_data User data
  * @retval See platform_err_t
  */
platform_err_t platform_device_pin_register(
    const char *name, const platform_pin_ops_t *ops, void *user_data)
{
    _hw_pin.parent.rx_indicate  = NULL;
    _hw_pin.parent.tx_complete  = NULL;

    _hw_pin.parent.ops          = &pin_ops;

    _hw_pin.ops                 = ops;
    _hw_pin.parent.user_data    = user_data;

    /* register a character device */
    return platform_device_register(&_hw_pin.parent, name,
                                    PLATFORM_DEVICE_FLAG_STANDALONE);
}


/**
  * @brief  Attach IQR handler to specified pin
  * @param  pin  Pin number
  * @param  mode IRQ mode
  * @param  hdr  IRQ callback function
  * @retval See platform_err_t
  */
platform_err_t platform_pin_attach_irq(int32_t pin, uint32_t mode,
                                       void (*hdr)(void *args), void  *args)
{
    platform_err_t ret = PLATFORM_ERROR;

    platform_assert(_hw_pin.ops != NULL);
    if (_hw_pin.ops->pin_attach_irq)
    {
        ret = _hw_pin.ops->pin_attach_irq(&_hw_pin.parent, pin, mode, hdr, args);
        goto exit;
    }
    ret = PLATFORM_ENOSYS;

exit:
    return ret;
}

/**
  * @brief  Dettach IQR handler to specified pin
  * @param  pin  Pin number
  * @retval See platform_err_t
  */
platform_err_t platform_pin_detach_irq(int32_t pin)
{
    platform_err_t ret = PLATFORM_ERROR;

    platform_assert(_hw_pin.ops != NULL);
    if (_hw_pin.ops->pin_detach_irq)
    {
        ret = _hw_pin.ops->pin_detach_irq(&_hw_pin.parent, pin);
        goto exit;
    }
    ret = PLATFORM_ENOSYS;

exit:
    return ret;
}

/**
  * @brief  Enable/Disable specified pin IRQ
  * @param  pin     Pin number
  * @param  enabled PIN_IRQ_ENABLE/PIN_IRQ_DISABLE
  * @retval See platform_err_t
  */
platform_err_t platform_pin_irq_enable(
    int32_t pin, uint32_t enabled)
{
    platform_err_t ret = PLATFORM_ERROR;
    platform_assert(_hw_pin.ops != NULL);
    if (_hw_pin.ops->pin_irq_enable)
    {
        ret = _hw_pin.ops->pin_irq_enable(&_hw_pin.parent, pin, enabled);
        goto exit;
    }
    ret = PLATFORM_ENOSYS;

exit:
    return ret;
}

/**
  * @brief  Set pin work mode
  * @param  pin  Pin number
  * @param  mode Work mode
  * @retval None
  */
void platform_pin_mode(int32_t pin, int32_t mode)
{
    platform_assert(_hw_pin.ops != NULL);
    _hw_pin.ops->pin_mode(&_hw_pin.parent, pin, mode);
}

/**
  * @brief  Write pin output logic level
  * @param  pin   Pin number
  * @param  value Output logic level
  * @retval None
  */
void platform_pin_write(int32_t pin, int32_t value)
{
    platform_assert(_hw_pin.ops != NULL);
    _hw_pin.ops->pin_write(&_hw_pin.parent, pin, value);
}

/**
  * @brief  Read pin input logic level
  * @param  pin Pin number
  * @retval Pin read value
  */
int32_t platform_pin_read(int32_t pin)
{
    platform_assert(_hw_pin.ops != NULL);
    return _hw_pin.ops->pin_read(&_hw_pin.parent, pin);
}

/**
  * @brief  Get pin number
  * @param  name Pin descriptor
  * @retval Pin number or error code
  */
int32_t platform_pin_get(const char *name)
{
    int32_t ret = -1;

    platform_assert(_hw_pin.ops != NULL);
    platform_assert(name[0] == 'P');

    if (NULL == _hw_pin.ops->pin_get)
    {
        ret = PLATFORM_ENOSYS;
        goto exit;
    }

    ret = _hw_pin.ops->pin_get(name);

exit:
    return ret;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  The implementation of the superclass interface - read
  * @param  dev    Platform device handle
  * @param  pos    Data position
  * @param  buffer Data Buffer pointer
  * @param  size   Data Length
  * @retval Actual read data length
  */
static uint32_t _pin_read(
    struct platform_device *dev, uint32_t pos, void *buffer, uint32_t size)
{
    uint32_t ret = 0;
    platform_device_pin_status_t *status;
    platform_device_pin_t *pin = ( platform_device_pin_t *)dev;

    /* check parameters */
    platform_assert(pin != NULL);

    status = (platform_device_pin_status_t *) buffer;
    if ((NULL == status) || (size != sizeof(*status)))
    {
        ret = 0;
        goto exit;
    }

    status->status = pin->ops->pin_read(dev, status->pin);
    ret = size;

exit:
    return ret;
}

/**
  * @brief  The implementation of the superclass interface - write
  * @param  dev    Platform device handle
  * @param  pos    Data position
  * @param  buffer Data Buffer pointer
  * @param  size   Data Length
  * @retval Actual write data length
  */
static uint32_t _pin_write(
    struct platform_device *dev, uint32_t pos,
    const void *buffer, uint32_t size)
{
    uint32_t ret = 0;
    platform_device_pin_status_t *status;
    platform_device_pin_t *pin = ( platform_device_pin_t *)dev;

    /* check parameters */
    platform_assert(pin != NULL);

    status = (platform_device_pin_status_t *) buffer;
    if ((NULL == status) || (size != sizeof(*status)))
    {
        ret = 0;
        goto exit;
    }

    pin->ops->pin_write(dev, (int32_t)status->pin, (int32_t)status->status);

    ret = size;

exit:
    return ret;
}

/**
  * @brief  The implementation of the superclass interface - control
  * @param  dev  Platform device handle
  * @param  cmd  Command ID
  * @param  args Passing args
  * @retval See platform_err_t
  */
static platform_err_t _pin_control(
    struct platform_device *dev, int cmd, void *args)
{
    platform_err_t ret = PLATFORM_ERROR;
    platform_device_pin_mode_t *mode;
    platform_device_pin_t *pin = ( platform_device_pin_t *)dev;

    /* check parameters */
    platform_assert(pin != NULL);

    mode = (platform_device_pin_mode_t *) args;
    if (NULL == mode)
    {
        ret = PLATFORM_ERROR;
        goto exit;
    }

    pin->ops->pin_mode(dev, (int32_t)mode->pin, (int32_t)mode->mode);

    ret = PLATFORM_EOK;

exit:
    return ret;
}

/******************** END OF FILE ******************END OF FILE****/
