/**
  ******************************************************************************
  * @file    led_gpio.c
  * @brief   The implementation of led_gpio class
  *
  * @details GPIO LED 子类. 见第 19 章 § 19.1. 子类只调 platform_pin 封装函数,
  *          永远不直接碰 GPIO 寄存器, 也不 include 厂家 HAL 头文件. 这就是
  *          第 15 章 "Platform 抽象到底" 的核心纪律: 跨芯片移植时这一份代码
  *          0 改动, 只换 platform_pin 子类即可 (STM32 HAL / libgpiod / sysfs / 模拟).
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "led_gpio.h"
#include "platform_pin.h"

/* Private function prototypes -----------------------------------------------*/
static void led_gpio_on(led_base_t *me);
static void led_gpio_off(led_base_t *me);

/* Private variables ---------------------------------------------------------*/
/* ops 表用 static + 一份共享: 同一份地址给该子类所有实例, 省 RAM. */
static led_base_ops_t ops =
{
     .led_on = led_gpio_on,
     .led_off = led_gpio_off,
};

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  constructor function
  * @param  me - this pointer
  * @param  pin_name - platform pin dev name, eg "PI.15"
  * @param  light_level - led_on output level
  * @retval see platform_err_t
  *
  * 构造函数 (类似 C++ ctor): 静态分配 + 显式 init 是嵌入式 OOP 的标准做法,
  * 不用 malloc 是因为内存可控 + 启动期可静态布局. 末尾 me->base.ops = &ops
  * 这一句把 vptr 装上, 之后 led_on(me)/led_off(me) 才能正确分发到本子类.
  */
platform_err_t led_gpio_init
(led_gpio_t *me, const char *pin_name, bool light_level)
{
    platform_err_t ret = PLATFORM_EOK;

    if((NULL == me) || (NULL == pin_name))
    {
        ret = PLATFORM_EINVAL;
        goto exit;
    }

    /* 字符串名解析成 pin 号. 字符串语义跨芯片一致 ("PA.5" / "GPIO17"...),
     * 看不到底层 port 索引或寄存器, 见第 15 章 "Platform 抽象到底". */
    int32_t pin_num = platform_pin_get(pin_name);

    if (pin_num < 0)
    {
        ret = PLATFORM_EINVAL;
        goto exit;
    }

    platform_pin_mode(pin_num, PIN_MODE_OUTPUT);
    platform_pin_write(pin_num, !light_level);  /* 初始为熄灭状态 */

    me->pin_num = pin_num;
    me->light_level = light_level;
    me->base.ops = &ops;        /* 装 vptr: 后续多态分发的关键一步 */

exit:
    return ret;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  turn on led
  * @param  me - base this pointer
  * @retval none
  *
  * 子类实现: me 形参类型是基类指针, 函数体里直接 cast 回子类指针拿私有
  * 字段. 因为 led_base 在 led_gpio 的第一字段, 两者地址重合, cast 是 0
  * 偏移; 基类不在第一字段时要用 container_of (见第 13 章).
  */
static void led_gpio_on(led_base_t *me)
{
    led_gpio_t *led_gpio = (led_gpio_t *)me;
    platform_pin_write(led_gpio->pin_num, led_gpio->light_level);
}

/**
  * @brief  turn off led
  * @param  me - base this pointer
  * @retval none
  */
static void led_gpio_off(led_base_t *me)
{
    led_gpio_t *led_gpio = (led_gpio_t *)me;
    platform_pin_write(led_gpio->pin_num, !led_gpio->light_level);
}

/******************** END OF FILE ******************END OF FILE****/
