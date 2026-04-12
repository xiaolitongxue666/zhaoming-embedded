/**
 * @file     led.c
 * @brief    LED模块实现 - led.h + led.c = 一个完整的"类"
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP08 - 你用C手搓了一个class
 *
 * EP08相比EP07的进化：
 *   1. 新增 initialized 标志位，防止没调init就使用
 *   2. 新增 pin 范围检查（0 ~ LED_PIN_MAX）
 *   3. init做三件事：参数验证 + 硬件配置 + 默认状态（= 构造函数）
 *   4. deinit做两件事：关闭硬件 + 释放资源（= 析构函数）
 *
 * C++ 写法：  Led led(pin);        // 构造函数
 *             led.on();            // 成员函数
 *             // 离开作用域自动析构
 *
 * C   写法：  Led_t led;
 *             led_init(&led, pin); // 构造函数
 *             led_on(&led);        // 成员函数
 *             led_deinit(&led);    // 析构函数（手动调用）
 *
 * 语法不同，思想完全一样。
 */

#include "led.h"
#include <stdio.h>

/* ============================================================
 *  static辅助函数（= C++的private方法）
 * ============================================================ */

/**
 * @brief  统一更新硬件状态
 * @note   static = private，外部调不到
 */
static void update_hardware(Led_t *me)
{
    platform_gpio_write(me->pin, me->is_on);
}

/**
 * @brief  检查亮度值是否合法
 * @return true=合法(0~100), false=超范围
 */
static bool is_brightness_valid(uint8_t brightness)
{
    return (brightness <= 100);
}

/**
 * @brief  检查引脚号是否合法
 * @return true=合法(0~LED_PIN_MAX), false=超范围
 */
static bool is_pin_valid(uint8_t pin)
{
    return (pin <= LED_PIN_MAX);
}

/* ============================================================
 *  公共函数实现（= C++的public方法）
 * ============================================================ */

/* ---- 构造 / 析构 ---- */

int led_init(Led_t *me, uint8_t pin)
{
    /* 第一件事：参数验证 */
    if (me == NULL) {
        return -1;
    }

    if (!is_pin_valid(pin)) {
        printf("  [LED] Error: Pin %d out of range (0~%d)\n", pin, LED_PIN_MAX);
        return -2;
    }

    /* 第二件事：硬件配置 */
    platform_gpio_init(pin, GPIO_MODE_OUTPUT);

    /* 第三件事：设置默认状态 */
    me->pin = pin;
    me->brightness = 0;
    me->is_on = false;
    me->initialized = true;    /* 标记：已初始化 */

    /* 确保上电后LED是灭的 */
    update_hardware(me);

    printf("  [LED] Pin %d initialized (open for business)\n", pin);
    return 0;
}

int led_deinit(Led_t *me)
{
    if (me == NULL) {
        return -1;
    }

    /* 第一步：关闭硬件（确保安全） */
    me->is_on = false;
    update_hardware(me);

    /* 第二步：释放GPIO资源 */
    platform_gpio_deinit(me->pin);

    /* 清除所有状态 */
    me->brightness = 0;
    me->initialized = false;   /* 标记：已反初始化 */

    printf("  [LED] Pin %d released (closed)\n", me->pin);
    return 0;
}

/* ---- 开 / 关 / 翻转 ---- */

int led_on(Led_t *me)
{
    if (me == NULL) {
        return -1;
    }

    if (!me->initialized) {
        printf("  [LED] Error: not initialized, call led_init() first!\n");
        return -3;
    }

    me->is_on = true;
    update_hardware(me);

    printf("  [LED] Pin %d ON\n", me->pin);
    return 0;
}

int led_off(Led_t *me)
{
    if (me == NULL) {
        return -1;
    }

    if (!me->initialized) {
        printf("  [LED] Error: not initialized, call led_init() first!\n");
        return -3;
    }

    me->is_on = false;
    update_hardware(me);

    printf("  [LED] Pin %d OFF\n", me->pin);
    return 0;
}

int led_toggle(Led_t *me)
{
    if (me == NULL) {
        return -1;
    }

    if (!me->initialized) {
        printf("  [LED] Error: not initialized, call led_init() first!\n");
        return -3;
    }

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

    if (!me->initialized) {
        printf("  [LED] Error: not initialized, call led_init() first!\n");
        return -3;
    }

    if (!is_brightness_valid(brightness)) {
        printf("  [LED] Error: brightness %d out of range (0~100)\n", brightness);
        return -2;
    }

    me->brightness = brightness;

    if (brightness == 0) {
        me->is_on = false;
    } else {
        me->is_on = true;
    }

    update_hardware(me);

    printf("  [LED] Pin %d brightness set to %d%%\n", me->pin, brightness);
    return 0;
}

/* ---- 状态查询 ---- */

int led_get_state(const Led_t *me, bool *is_on, uint8_t *brightness)
{
    if (me == NULL) {
        return -1;
    }

    if (is_on != NULL) {
        *is_on = me->is_on;
    }

    if (brightness != NULL) {
        *brightness = me->brightness;
    }

    return 0;
}
