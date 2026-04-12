/**
 * @file     main.c
 * @brief    EP06演示 - 三颗LED，一份代码
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP06 - 封装：struct与me指针
 *
 * 这个程序演示的核心问题和解决方案：
 *
 *   问题：三个LED（红灯pin13、绿灯pin14、蓝灯pin15），
 *         以前要写三份几乎一模一样的代码。
 *
 *   解决：用struct把数据打包成Led_t，
 *         用me指针让同一份函数服务不同的数据。
 *
 *   一句话：封装不是藏代码 —— 是让同一份逻辑服务不同的数据。
 */

#include <stdio.h>
#include "led.h"

int main(void)
{
    printf("========================================\n");
    printf("  EP06 Encapsulation: struct & me pointer\n");
    printf("  \"Same code, different data\"\n");
    printf("========================================\n\n");

    /*
     * 第一步：创建三个LED"对象"
     * 每个Led_t变量就是一颗LED的全部数据，
     * 就像三张不同的身份证。
     */
    Led_t red_led;      /* 红灯 - 引脚13 */
    Led_t green_led;    /* 绿灯 - 引脚14 */
    Led_t blue_led;     /* 蓝灯 - 引脚15 */

    /* ---- 初始化三颗LED ---- */
    printf("--- Init ---\n");
    led_init(&red_led,   13);
    led_init(&green_led, 14);
    led_init(&blue_led,  15);

    /*
     * 第二步：分别操作三颗LED
     * 注意：led_on() 只有一份代码，
     * 传不同的me指针，就操作不同的LED。
     *
     * 如果是C++，写法是：  red_led.on();
     * 我们在C里写成：      led_on(&red_led);
     * 区别只是this变成了me，由我们手动传。
     */
    printf("\n--- Turn on RED ---\n");
    led_on(&red_led);

    printf("\n--- Turn on GREEN ---\n");
    led_on(&green_led);

    printf("\n--- Turn on BLUE ---\n");
    led_on(&blue_led);

    /* ---- 关掉红灯 ---- */
    printf("\n--- Turn off RED ---\n");
    led_off(&red_led);

    /* ---- 翻转绿灯（亮变灭） ---- */
    printf("\n--- Toggle GREEN ---\n");
    led_toggle(&green_led);

    /* ---- 设置蓝灯亮度 ---- */
    printf("\n--- Set BLUE brightness to 75%% ---\n");
    led_set_brightness(&blue_led, 75);

    printf("\n--- Set BLUE brightness to 0%% (auto off) ---\n");
    led_set_brightness(&blue_led, 0);

    /* ---- 故意测试亮度超范围 ---- */
    printf("\n--- Test brightness out of range (200) ---\n");
    led_set_brightness(&blue_led, 200);

    /*
     * 第三步：清理资源
     * 好习惯：用完了要释放，对应init/deinit。
     */
    printf("\n--- Cleanup ---\n");
    led_deinit(&red_led);
    led_deinit(&green_led);
    led_deinit(&blue_led);

    printf("\n========================================\n");
    printf("  Summary: 3 LEDs, 1 set of led_on/off/toggle code.\n");
    printf("  me pointer decides who to operate -- that is encapsulation.\n");
    printf("========================================\n");

    printf("\nPress Enter to exit...\n");
    getchar();
    return 0;
}
