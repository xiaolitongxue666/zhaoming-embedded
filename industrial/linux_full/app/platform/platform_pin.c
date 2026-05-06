/**
  ******************************************************************************
  * @file    platform_pin.c
  * @brief   PIN device driver framework (上层封装函数 + ops 分发).
  *
  * 这一份是 platform 层"对外封装、对内 ops"的标准结构: 维护一个 static
  * ops 指针 _g_ops, 上层调 platform_pin_xxx 时内部 dispatch. 子类启动期
  * 调 platform_pin_register(&_xxx_pin_ops) 把指针填进来.
  ******************************************************************************
  */

#include "platform/platform_pin.h"
#include "platform/platform_assert.h"

#include <stddef.h>

static const platform_pin_ops_t *_g_ops = NULL;

/**
  * @brief  Pin device register.
  * @param  ops  Subclass ops table.
  * @retval See platform_err_t.
  */
platform_err_t platform_pin_register(const platform_pin_ops_t *ops)
{
    platform_err_t ret = PLATFORM_EINVAL;

    if (NULL == ops)
    {
        goto exit;
    }

    _g_ops = ops;
    ret = PLATFORM_EOK;

exit:
    return ret;
}

/**
  * @brief  Set pin work mode.
  * @param  pin   Pin number.
  * @param  mode  Pin mode (PIN_MODE_OUTPUT / PIN_MODE_INPUT / ...).
  */
void platform_pin_mode(int32_t pin, int32_t mode)
{
    platform_assert(_g_ops != NULL);
    platform_assert(_g_ops->pin_mode != NULL);
    _g_ops->pin_mode(pin, mode);
}

/**
  * @brief  Write pin output logic level.
  * @param  pin    Pin number.
  * @param  value  PIN_LOW / PIN_HIGH.
  */
void platform_pin_write(int32_t pin, int32_t value)
{
    platform_assert(_g_ops != NULL);
    platform_assert(_g_ops->pin_write != NULL);
    _g_ops->pin_write(pin, value);
}

/**
  * @brief  Read pin input logic level.
  * @param  pin  Pin number.
  * @retval PIN_LOW / PIN_HIGH.
  */
int32_t platform_pin_read(int32_t pin)
{
    platform_assert(_g_ops != NULL);
    platform_assert(_g_ops->pin_read != NULL);
    return _g_ops->pin_read(pin);
}

/**
  * @brief  Attach IRQ handler to specified pin.
  */
platform_err_t platform_pin_attach_irq(
    int32_t pin, uint32_t mode, void (*hdr)(void *args), void *args)
{
    platform_err_t ret = PLATFORM_ERROR;

    platform_assert(_g_ops != NULL);
    if (_g_ops->pin_attach_irq)
    {
        ret = _g_ops->pin_attach_irq(pin, mode, hdr, args);
        goto exit;
    }
    ret = PLATFORM_ENOSYS;

exit:
    return ret;
}

/**
  * @brief  Detach IRQ handler from specified pin.
  */
platform_err_t platform_pin_detach_irq(int32_t pin)
{
    platform_err_t ret = PLATFORM_ERROR;

    platform_assert(_g_ops != NULL);
    if (_g_ops->pin_detach_irq)
    {
        ret = _g_ops->pin_detach_irq(pin);
        goto exit;
    }
    ret = PLATFORM_ENOSYS;

exit:
    return ret;
}

/**
  * @brief  Enable / disable specified pin IRQ.
  */
platform_err_t platform_pin_irq_enable(int32_t pin, uint32_t enabled)
{
    platform_err_t ret = PLATFORM_ERROR;

    platform_assert(_g_ops != NULL);
    if (_g_ops->pin_irq_enable)
    {
        ret = _g_ops->pin_irq_enable(pin, enabled);
        goto exit;
    }
    ret = PLATFORM_ENOSYS;

exit:
    return ret;
}

/**
  * @brief  Resolve pin name (eg. "PA.5", "PD.12", "PI.14") to pin number.
  * @param  name  Pin descriptor.
  * @retval Pin number, or negative on error.
  */
int32_t platform_pin_get(const char *name)
{
    int32_t ret = -1;

    platform_assert(_g_ops != NULL);
    platform_assert(name != NULL);
    platform_assert(name[0] == 'P');

    if (NULL == _g_ops->pin_get)
    {
        ret = PLATFORM_ENOSYS;
        goto exit;
    }

    ret = _g_ops->pin_get(name);

exit:
    return ret;
}

/******************** END OF FILE ********************/
