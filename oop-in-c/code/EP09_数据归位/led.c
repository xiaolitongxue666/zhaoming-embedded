/**
 * @file     led.c
 * @brief    LED模块实现 - 没有一个裸露的全局变量
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP09 - 你的全局变量，该死了
 *
 * EP09 数据归位改造：
 *
 *   【改造前】.c文件开头5个裸露的全局变量
 *     int g_pin = 0;
 *     int g_brightness = 0;
 *     int init_count = 0;
 *     int MAX_BRIGHTNESS = 255;
 *     int g_debug_flag = 0;
 *
 *   【改造后】每一份数据都有主人
 *     pin, brightness   → struct成员（跟着me走）
 *     init_count        → static int（文件私有，外面看不到）
 *     debug_flag        → static int（文件私有，外面看不到）
 *     MAX_BRIGHTNESS    → static const（只读，不可修改）
 *
 *   C vs C++ 对照：
 *     static int s_init_count   ←→  class Led { static int init_count; }
 *     static const MAX_BRIGHTNESS ←→  static const 类成员
 *     static void clamp_brightness() ←→ private方法
 *
 *   金句：数据没有主人，bug就是主人。
 */

#include "led.h"
#include <stdio.h>

/* ============================================================
 *  第三类：只读常量 → static const（替代#define，有类型检查）
 *
 *  为什么不用 #define MAX_BRIGHTNESS 255？
 *    - #define 是文本替换，没有类型
 *    - static const 有类型，编译器帮你检查
 *    - 调试时能看到变量名，#define看不到
 * ============================================================ */

static const uint8_t MAX_BRIGHTNESS = 100;  /* 亮度上限（只读，不可修改） */
static const uint8_t MAX_PIN        = 15;   /* 引脚上限（只读，不可修改） */

/* ============================================================
 *  第二类：模块内部共享数据 → static变量（文件私有）
 *
 *  这些变量属于整个LED模块，不属于某一个LED实例。
 *  加了static，外部文件看不到 —— 不怕被别人乱改。
 *
 *  C vs C++：
 *    static int s_init_count ←→ class Led { static int init_count; }
 *    效果一模一样：属于类，不属于某个对象。
 * ============================================================ */

static int s_init_count = 0;   /* 模块累计初始化次数（文件私有） */
static int s_debug_flag = 0;   /* 模块调试开关（文件私有） */

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
 * @return true=合法(0~MAX_BRIGHTNESS), false=超范围
 */
static bool is_brightness_valid(uint8_t brightness)
{
    return (brightness <= MAX_BRIGHTNESS);
}

/**
 * @brief  检查引脚号是否合法
 * @return true=合法(0~MAX_PIN), false=超范围
 */
static bool is_pin_valid(uint8_t pin)
{
    return (pin <= MAX_PIN);
}

/**
 * @brief  调试打印（受s_debug_flag控制）
 * @note   static函数 —— 文件私有，外部调不到
 *
 * static的第一层含义：函数前加static = 文件私有函数
 */
static void debug_print(const char *msg)
{
    if (s_debug_flag) {
        printf("  [LED-DEBUG] %s\n", msg);
    }
}

/* ============================================================
 *  公共函数实现
 *
 *  第一类数据：pin, brightness → struct成员（跟着me走）
 *  每个LED对象有自己的pin和brightness，互不干扰。
 * ============================================================ */

int led_init(Led_t *me, uint8_t pin)
{
    if (me == NULL) {
        return -1;
    }

    if (!is_pin_valid(pin)) {
        printf("  [LED] Error: Pin %d out of valid range (0~%d)\n", pin, MAX_PIN);
        return -2;
    }

    /* 硬件配置 */
    platform_gpio_init(pin, GPIO_MODE_OUTPUT);

    /* 实例数据 → 存进struct，跟着me走 */
    me->pin = pin;             /* 不是g_pin，是me->pin！ */
    me->brightness = 0;        /* 不是g_brightness，是me->brightness！ */
    me->is_on = false;
    me->initialized = true;

    /* 确保上电后LED是灭的 */
    update_hardware(me);

    /* 模块级数据：init次数 +1 */
    s_init_count++;

    debug_print("init done");
    printf("  [LED] Pin %d initialized (init #%d)\n", pin, s_init_count);
    return 0;
}

int led_deinit(Led_t *me)
{
    if (me == NULL) {
        return -1;
    }

    /* 关闭硬件 */
    me->is_on = false;
    update_hardware(me);

    /* 释放GPIO */
    platform_gpio_deinit(me->pin);

    /* 清除状态 */
    me->brightness = 0;
    me->initialized = false;

    debug_print("deinit done");
    printf("  [LED] Pin %d released\n", me->pin);
    return 0;
}

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
        printf("  [LED] Error: brightness %d out of range (0~%d)\n", brightness, MAX_BRIGHTNESS);
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

/**
 * @brief  获取模块累计初始化次数
 *
 * s_init_count是static变量，外部不能直接访问。
 * 想知道init了几次？通过这个函数问 —— 数据的主人说了算。
 *
 * 这就是封装：你可以看，但得经过我同意。
 */
int led_get_init_count(void)
{
    return s_init_count;
}
