/**
 * @file     led_bad.c
 * @brief    LED模块（反面教材）- 全局变量满天飞
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP09 - 你的全局变量，该死了
 *
 * 【反面教材 —— 请勿模仿！】
 *
 *   看看这个文件开头：5个全局变量，像路边摊一样摆在那里，
 *   谁都能碰，谁都能改。
 *
 *   致命问题：
 *     两个LED（红灯pin=5，绿灯pin=3）共享同一个 g_pin。
 *     初始化绿灯时，g_pin从5变成3 → 红灯的pin被覆盖！
 *     之后操作红灯，实际操作的是绿灯的引脚。
 *
 *   数据没有主人，bug就是主人。
 */

#include "led_bad.h"
#include <stdio.h>

/* ============================================================
 *  反面教材：5个全局变量散在文件开头
 *
 *  问题1：g_pin / g_brightness —— 实例数据用全局变量
 *         两个LED共享同一个g_pin，第二次init覆盖第一次
 *
 *  问题2：init_count / g_debug_flag —— 模块数据没加static
 *         外部文件可以直接 extern int init_count; 随便改
 *
 *  问题3：MAX_BRIGHTNESS —— 用int变量存常量
 *         运行时可以被修改，不安全
 * ============================================================ */

int g_pin = 0;              /* 当前引脚号（全局！任何人都能改！） */
int g_brightness = 0;       /* 当前亮度（全局！） */
int init_count = 0;         /* 初始化次数（没加static，外部可见！） */
int MAX_BRIGHTNESS = 255;   /* 最大亮度（用变量存常量？随时能被改！） */
int g_debug_flag = 0;       /* 调试开关（没加static，外部可见！） */

/* ============================================================
 *  函数实现
 * ============================================================ */

int bad_led_init(uint8_t pin)
{
    /* g_pin是全局的，每次init都会覆盖！ */
    g_pin = pin;
    g_brightness = 0;
    init_count++;

    platform_gpio_init(pin, GPIO_MODE_OUTPUT);
    platform_gpio_write(pin, false);

    printf("  [BAD_LED] Pin %d initialized (g_pin=%d, init #%d)\n",
           pin, g_pin, init_count);
    return 0;
}

int bad_led_on(void)
{
    /* 用的是g_pin —— 如果被覆盖了，操作的就是别人的引脚！ */
    platform_gpio_write((uint8_t)g_pin, true);
    printf("  [BAD_LED] Pin %d ON\n", g_pin);
    return 0;
}

int bad_led_off(void)
{
    platform_gpio_write((uint8_t)g_pin, false);
    printf("  [BAD_LED] Pin %d OFF\n", g_pin);
    return 0;
}

int bad_led_set_brightness(uint8_t brightness)
{
    if (brightness > MAX_BRIGHTNESS) {
        printf("  [BAD_LED] Error: brightness %d out of range (0~%d)\n",
               brightness, MAX_BRIGHTNESS);
        return -1;
    }

    g_brightness = brightness;
    printf("  [BAD_LED] brightness set to %d (MAX=%d)\n", g_brightness, MAX_BRIGHTNESS);
    return 0;
}

int bad_led_get_pin(void)
{
    return g_pin;
}

int bad_led_get_brightness(void)
{
    return g_brightness;
}
