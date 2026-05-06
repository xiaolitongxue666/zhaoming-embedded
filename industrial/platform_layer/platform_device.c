/**
  ******************************************************************************
  * @file    platform_device.c
  * @brief   The implementation of platform device.
  *
  * @details 见 platform_device.h 文件头. 通用设备基类的实现. 维护一张
  *          MAX_PLATFORM_DEV_NUM 项的静态注册表, 设备按名注册 / 注销 / 查找.
  *          通用 init/open/close/read/write/control 接口体内做参数校验 +
  *          ref_count 引用计数 + mutex 临界保护 + ops 分发, 子类只需要实现
  *          自己的 read/write/control 几个虚函数即可.
  ******************************************************************************
  */

#include <platform/platform_assert.h>
#include <platform/platform_device.h>
#define LOG_TAG "DEVICE"
#include "elog.h"

typedef struct{
	uint8_t is_used;
	platform_device_t device;
}platform_dev_cell_t;

#define device_ops		(dev->ops)
#define device_init     (dev->ops->init)
#define device_open     (dev->ops->open)
#define device_close    (dev->ops->close)
#define device_read     (dev->ops->read)
#define device_write    (dev->ops->write)
#define device_control  (dev->ops->control)


static platform_dev_cell_t platform_dev_array[MAX_PLATFORM_DEV_NUM];
static uint32_t platform_dev_num = 0;

static platform_err_t insert_device_array(platform_device_t dev)
{
	platform_dev_cell_t *p_current_cell;
	
	if(platform_dev_num == MAX_PLATFORM_DEV_NUM) 
	{
		return PLATFORM_EFULL;
	}

	for(uint32_t i = 0; i < MAX_PLATFORM_DEV_NUM; i++)
	{
		p_current_cell = &platform_dev_array[i];
		if(p_current_cell->is_used == 0) break;
	}

	p_current_cell->is_used = 1;
	p_current_cell->device = dev;
	platform_dev_num++;
	return PLATFORM_EOK;
}

static platform_err_t remove_device_array(platform_device_t dev)
{
	platform_dev_cell_t *p_current_cell;
	for(uint32_t i = 0; i < MAX_PLATFORM_DEV_NUM; i++)
	{
		p_current_cell = &platform_dev_array[i];
		if(!strcmp(p_current_cell->device->name, dev->name))
		{
			p_current_cell->is_used = 0;
			platform_dev_num--;
			return PLATFORM_EOK;
		}
	}
	return PLATFORM_EINVAL;
}


/**
 * This function registers a device driver with specified name.
 *
 * @param dev the pointer of device driver structure
 * @param name the device driver's name
 * @param flags the capabilities flag of device
 *
 * @return the error code, PLATFORM_EOK on initialization successfully.
 */
platform_err_t platform_device_register(platform_device_t dev, const char *name, uint16_t flags)
{
	platform_err_t result;
	PLATFORM_SYS_ARCH_DECL_PROTECT(lev);
	platform_assert(dev != NULL);
    if (platform_device_find(name) != NULL)
    {
    	result = PLATFORM_ERROR;
    	goto exit;
    }
    PLATFORM_SYS_ARCH_PROTECT(lev);   
	dev->name = name;
	dev->flag = flags;
	dev->ref_count = 0;
    dev->open_flag = 0;
	result = platform_sys_mutex_new(&dev->dev_mutex);
	if(result != PLATFORM_EOK) goto exit;

	result = insert_device_array(dev);
exit:
	PLATFORM_SYS_ARCH_UNPROTECT(lev);
	return result;
}

/**
 * This function removes a previously registered device driver
 *
 * @param dev the pointer of device driver structure
 *
 * @return the error code, PLATFORM_EOK on successfully.
 */
platform_err_t platform_device_unregister(platform_device_t dev)
{
	platform_err_t result;
	PLATFORM_SYS_ARCH_DECL_PROTECT(lev);
	PLATFORM_SYS_ARCH_PROTECT(lev);
	platform_assert(dev != NULL);
	result = remove_device_array(dev);
	PLATFORM_SYS_ARCH_UNPROTECT(lev);
	return result;
}

/**
 * This function finds a device driver by specified name.
 *
 * @param name the device driver's name
 *
 * @return the registered device driver on successful, or NULL on failure.
 */
platform_device_t platform_device_find(const char *name)
{
	platform_dev_cell_t *p_current_cell;
	platform_device_t dev = NULL;
	PLATFORM_SYS_ARCH_DECL_PROTECT(lev);
	PLATFORM_SYS_ARCH_PROTECT(lev);
	for(uint32_t i = 0; i < MAX_PLATFORM_DEV_NUM; i++)
	{
		p_current_cell = &platform_dev_array[i];
		if(p_current_cell->is_used == 1)
		{
			if(!strcmp(p_current_cell->device->name, name))
			{
				dev =  p_current_cell->device;
				goto exit;
			}
				
		}
	}
exit:
	PLATFORM_SYS_ARCH_UNPROTECT(lev);
	return dev;
}

/**
 * This function will initialize the specified device
 *
 * @param dev the pointer of device driver structure
 *
 * @return the result
 */
platform_err_t platform_device_init(platform_device_t dev)
{
    platform_err_t result = PLATFORM_EOK;

    platform_assert(dev != NULL);
	platform_sys_mutex_lock(&dev->dev_mutex);

    /* get device_init handler */
    if (device_ops != NULL && device_init != NULL)
    {
        if (!(dev->open_flag & PLATFORM_DEVICE_OFLAG_INITIALIZED))
        {
            result = device_init(dev);
            if (result != PLATFORM_EOK)
            {
                log_e("To initialize device:%s failed. The error code is %d\n",
                           dev->name, (int)result);
            }
            else
            {
                dev->open_flag |= PLATFORM_DEVICE_OFLAG_INITIALIZED;
            }
        }
    }
	platform_sys_mutex_unlock(&dev->dev_mutex);
    return result;	
}

/**
 * This function will open a device
 *
 * @param dev the pointer of device driver structure
 *
 * @return the result
 */
platform_err_t platform_device_open(platform_device_t dev)
{
    platform_err_t result = PLATFORM_EOK;

	platform_sys_mutex_lock(&dev->dev_mutex);
    platform_assert(dev != NULL);
	
    /* if device is not initialized, initialize it. */
    if (!(dev->open_flag & PLATFORM_DEVICE_OFLAG_INITIALIZED))
    {
        if (device_ops != NULL && device_init != NULL)
        {
            result = device_init(dev);
            if (result != PLATFORM_EOK)
            {
                log_e("To initialize device:%s failed. The error code is %d\n",
                           dev->name, (int)result);
				goto exit;
            }
        }

        dev->open_flag |= PLATFORM_DEVICE_OFLAG_INITIALIZED;
    }

    /* device is a stand alone device and opened */
    if ((dev->flag & PLATFORM_DEVICE_FLAG_STANDALONE) &&
        (dev->open_flag & PLATFORM_DEVICE_OFLAG_OPEN))
    {
    	result = PLATFORM_EBUSY;
		goto exit;
    }

    /* call device_open interface */
    if (device_ops != NULL && device_open != NULL)
    {
        result = device_open(dev);
    }

    /* set open flag */
    if (result == PLATFORM_EOK || result == PLATFORM_ENOSYS)
    {
        dev->open_flag |= PLATFORM_DEVICE_OFLAG_OPEN;

        dev->ref_count++;
        /* don't let bad things happen silently. If you are bitten by this assert,
         * please set the ref_count to a bigger type. */
        platform_assert(dev->ref_count != 0);
    }
exit:
	platform_sys_mutex_unlock(&dev->dev_mutex);
    return result;
}

/**
 * This function will close a device
 *
 * @param dev the pointer of device driver structure
 *
 * @return the result
 */
platform_err_t platform_device_close(platform_device_t dev)
{
    platform_err_t result = PLATFORM_EOK;
	platform_sys_mutex_lock(&dev->dev_mutex);
    platform_assert(dev != NULL);

    if (dev->ref_count == 0)
    {
    	result = PLATFORM_ERROR;
		goto exit;
    }
		
    dev->ref_count--;

    if (dev->ref_count != 0)
    {
    	result = PLATFORM_EOK;
		goto exit;
    }
		
    /* call device_close interface */
    if (device_ops != NULL && device_close != NULL)
    {
        result = device_close(dev);
    }

    /* set open flag */
    if (result == PLATFORM_EOK || result == PLATFORM_ENOSYS)
        dev->open_flag = PLATFORM_DEVICE_OFLAG_CLOSE;
exit:
	platform_sys_mutex_unlock(&dev->dev_mutex);
    return result;
}

/**
 * This function will read some data from a device.
 *
 * @param dev the pointer of device driver structure
 * @param pos the position of reading
 * @param buffer the data buffer to save read data
 * @param size the size of buffer
 *
 * @return the actually read size on successful, otherwise negative returned.
 *
 */
uint32_t platform_device_read(platform_device_t dev,
                         uint32_t    pos,
                         void       *buffer,
                         uint32_t   size)
{
    platform_assert(dev != NULL);

    if (dev->ref_count == 0)
    {
        log_e("device %s read before open\n", dev->name);
        return 0;
    }

    /* call device_read interface */
    if (device_ops != NULL && device_read != NULL)
    {
        return device_read(dev, pos, buffer, size);
    }

    return 0;
}

/**
 * This function will write some data to a device.
 *
 * @param dev the pointer of device driver structure
 * @param pos the position of written
 * @param buffer the data buffer to be written to device
 * @param size the size of buffer
 *
 * @return the actually written size on successful, otherwise negative returned.
 *
 */
uint32_t platform_device_write(platform_device_t dev,
                          uint32_t    pos,
                          const void *buffer,
                          uint32_t   size)
{
    platform_assert(dev != NULL);

    if (dev->ref_count == 0)
    {
        log_e("device %s write before open\n", dev->name);
        return 0;
    }

    /* call device_write interface */
    if (device_ops != NULL && device_write != NULL)
    {
        return device_write(dev, pos, buffer, size);
    }

    return 0;
}

/**
 * This function will perform a variety of control functions on devices.
 *
 * @param dev the pointer of device driver structure
 * @param cmd the command sent to device
 * @param arg the argument of command
 *
 * @return the result
 */
platform_err_t platform_device_control(platform_device_t dev, int cmd, void *arg)
{
    platform_assert(dev != NULL);

    /* call device_write interface */
    if (device_ops != NULL && device_control != NULL)
    {
        return device_control(dev, cmd, arg);
    }

    return PLATFORM_ENOSYS;
}

/**
 * This function will set the reception indication callback function. This callback function
 * is invoked when this device receives data.
 *
 * @param dev the pointer of device driver structure
 * @param rx_ind the indication callback function
 *
 * @return PLATFORM_EOK
 */
platform_err_t
platform_device_set_rx_indicate(platform_device_t dev,
                          platform_err_t (*rx_ind)(platform_device_t dev, uint32_t size))
{
    platform_assert(dev != NULL);

    dev->rx_indicate = rx_ind;

    return PLATFORM_EOK;
}

/**
 * This function will set the indication callback function when device has
 * written data to physical hardware.
 *
 * @param dev the pointer of device driver structure
 * @param tx_done the indication callback function
 *
 * @return PLATFORM_EOK
 */
platform_err_t
platform_device_set_tx_complete(platform_device_t dev,
                          platform_err_t (*tx_done)(platform_device_t dev, void *buffer))
{
    platform_assert(dev != NULL);

    dev->tx_complete = tx_done;

    return PLATFORM_EOK;
}

/******************** END OF FILE ******************END OF FILE****/
