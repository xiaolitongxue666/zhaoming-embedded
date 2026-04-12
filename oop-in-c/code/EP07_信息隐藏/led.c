/**
 * @file     led.c
 * @brief    LED模块实现 - static函数 = 后厨，外面看不到也调不到
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP07 - 信息隐藏：static与头文件
 *
 * 关键理解：
 *   EP06里，led_on/off/set_brightness 都各自直接调 platform_gpio_write，
 *   GPIO操作散落在每个函数里。如果哪天硬件逻辑变了（比如高低电平反转），
 *   你得改好几个地方，漏改一个就出bug。
 *
 *   EP07的改进：
 *   1. 抽出 static void update_hardware() 集中管理GPIO操作
 *      —— 所有硬件细节只在这一个地方，改一处全生效
 *   2. static 修饰 = 这个函数只在 led.c 内部可见
 *      —— 就像后厨，顾客（外部代码）进不来
 *
 *   C++ 用 private: 标记私有方法；
 *   C 用 static 修饰函数 —— 效果一样，语法不同。
 */

#include "led.h"
#include <stdio.h>

/* ============================================================
 *  static辅助函数（后厨——外部看不到，也调不到）
 * ============================================================ */

/**
 * @brief  统一更新硬件状态（集中管理GPIO操作）
 *
 * 为什么要集中管理？
 *   以前 led_on 里写一遍 gpio_write，led_off 里又写一遍，
 *   如果有一天硬件逻辑变了（比如低电平亮灯），你得改好几个地方。
 *   现在只改这一个函数就够了。
 *
 * 为什么是 static？
 *   这是内部实现细节，外部不需要知道、也不应该直接调用。
 *   static 让它只在本文件可见 —— C语言的 private。
 */
static void update_hardware(Led_t *me)
{
    platform_gpio_write(me->pin, me->is_on);
}

/**
 * @brief  检查亮度值是否合法
 * @return true=合法, false=超范围
 *
 * 同样是 static —— 这种小工具函数属于内部实现，不暴露给外部。
 */
static bool is_brightness_valid(uint8_t brightness)
{
    return (brightness <= 100);
}

/* ============================================================
 *  公共函数实现（菜单上的菜——外部可以调用）
 * ============================================================ */

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

    /* 确保上电后LED是灭的 —— 通过 update_hardware 统一操作 */
    update_hardware(me);

    printf("  [LED] Pin %d LED initialized\n", pin);
    return 0;
}

int led_deinit(Led_t *me)
{
    if (me == NULL) {
        return -1;
    }

    /* 先关灯 */
    me->is_on = false;
    update_hardware(me);

    /* 再释放引脚 */
    platform_gpio_deinit(me->pin);

    /* 清除状态 */
    me->brightness = 0;

    printf("  [LED] Pin %d LED released\n", me->pin);
    return 0;
}

/* ---- 开 / 关 / 翻转 ---- */

int led_on(Led_t *me)
{
    if (me == NULL) {
        return -1;
    }

    me->is_on = true;
    update_hardware(me);    /* 集中管理硬件操作 */

    printf("  [LED] Pin %d ON\n", me->pin);
    return 0;
}

int led_off(Led_t *me)
{
    if (me == NULL) {
        return -1;
    }

    me->is_on = false;
    update_hardware(me);    /* 集中管理硬件操作 */

    printf("  [LED] Pin %d OFF\n", me->pin);
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

    /* 用 static 辅助函数检查亮度范围 */
    if (!is_brightness_valid(brightness)) {
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
    } else {
        me->is_on = true;
    }

    update_hardware(me);    /* 统一走这里操作硬件 */

    printf("  [LED] Pin %d brightness set to %d%%\n", me->pin, brightness);
    return 0;
}

/* ---- 状态查询 ---- */

int led_get_state(const Led_t *me, bool *is_on, uint8_t *brightness)
{
    if (me == NULL) {
        return -1;
    }

    /* 输出参数可以传NULL，表示"我不需要这个信息" */
    if (is_on != NULL) {
        *is_on = me->is_on;
    }

    if (brightness != NULL) {
        *brightness = me->brightness;
    }

    return 0;
}
