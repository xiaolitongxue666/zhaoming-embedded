/**
 * @file     led.c
 * @brief    LED模块实现 - 每个函数第一个参数都是 Led_t *me
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP06 - 封装：struct与me指针
 *
 * 关键理解：
 *   以前三颗LED要写三份代码（red_on, green_on, blue_on），
 *   现在只需要一份 led_on(me) —— me指向谁，就操作谁。
 *   这就是"同一份逻辑，服务不同的数据"。
 *
 *   C++ 里写 led.on()，编译器偷偷传了一个 this 指针；
 *   我们在C里手动传 me，效果完全一样。
 */

#include "led.h"
#include <stdio.h>

/* ---- 初始化 / 反初始化 ---- */

int led_init(Led_t *me, uint8_t pin)
{
    if (me == NULL) {
        return -1;
    }

    /* 记录引脚号，设置初始状态 */
    me->pin = pin;
    me->brightness = 0;
    me->is_on = false;

    /* 调用平台层，初始化硬件引脚为输出模式 */
    platform_gpio_init(pin, GPIO_MODE_OUTPUT);

    /* 确保上电后LED是灭的 */
    platform_gpio_write(pin, false);

    printf("  [LED] Pin%d LED initialized\n", pin);
    return 0;
}

int led_deinit(Led_t *me)
{
    if (me == NULL) {
        return -1;
    }

    /* 先关灯，再释放引脚 */
    platform_gpio_write(me->pin, false);
    platform_gpio_deinit(me->pin);

    /* 清除状态 */
    me->is_on = false;
    me->brightness = 0;

    printf("  [LED] Pin%d LED released\n", me->pin);
    return 0;
}

/* ---- 开 / 关 / 翻转 ---- */

int led_on(Led_t *me)
{
    if (me == NULL) {
        return -1;
    }

    me->is_on = true;
    platform_gpio_write(me->pin, true);

    printf("  [LED] Pin%d ON\n", me->pin);
    return 0;
}

int led_off(Led_t *me)
{
    if (me == NULL) {
        return -1;
    }

    me->is_on = false;
    platform_gpio_write(me->pin, false);

    printf("  [LED] Pin%d OFF\n", me->pin);
    return 0;
}

int led_toggle(Led_t *me)
{
    if (me == NULL) {
        return -1;
    }

    /* 当前亮就灭，当前灭就亮 */
    if (me->is_on) {
        led_off(me);
    } else {
        led_on(me);
    }

    return 0;
}

/* ---- 亮度 ---- */

int led_set_brightness(Led_t *me, uint8_t brightness)
{
    if (me == NULL) {
        return -1;
    }

    if (brightness > 100) {
        printf("  [LED] Error: brightness %d out of range (0~100)\n", brightness);
        return -2;
    }

    me->brightness = brightness;

    /*
     * 真实硬件上这里会配置PWM占空比来调节亮度。
     * PC模拟环境下，我们用打印来展示效果。
     * 亮度为0时自动关灯，大于0时自动开灯。
     */
    if (brightness == 0) {
        me->is_on = false;
        platform_gpio_write(me->pin, false);
    } else {
        me->is_on = true;
        platform_gpio_write(me->pin, true);
    }

    printf("  [LED] Pin%d brightness set to %d%%\n", me->pin, brightness);
    return 0;
}
