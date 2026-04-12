/**
 * @file     main.c
 * @brief    EP09演示 - 全局变量的罪与罚，数据归位的三种策略
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP09 - 你的全局变量，该死了
 *
 * 这个程序分三部分演示：
 *
 *   第一部分：反面教材（led_bad）
 *     两个LED共享g_pin，初始化绿灯后红灯的pin被覆盖 → bug！
 *
 *   第二部分：正面教材（led）
 *     两个LED各有自己的struct，数据跟着me走，互不干扰。
 *
 *   第三部分：模块级数据
 *     init_count通过函数获取，不暴露变量 → 封装闭环。
 *
 *   金句：数据没有主人，bug就是主人。
 */

#include <stdio.h>
#include "led_bad.h"
#include "led.h"

int main(void)
{
    printf("========================================\n");
    printf("  EP09 Your Global Variables Must Die\n");
    printf("  \"No owner for data, bug is the owner.\"\n");
    printf("========================================\n\n");

    /* ==============================================================
     *  第一部分：反面教材 —— 全局变量的罪
     *
     *  场景：你有两个LED，红灯pin=5，绿灯pin=3。
     *  问题：它们共享同一个 g_pin 全局变量！
     *        初始化绿灯时，g_pin 从5变成3，
     *        之后操作红灯 → 实际操作的是引脚3（绿灯的引脚）！
     *
     *  这个bug非常隐蔽：代码不报错，逻辑全错。
     * ============================================================== */
    printf("=== Part 1: Bad Example (The Sin of Global Variables) ===\n\n");

    printf("--- Init red LED (pin=5) ---\n");
    bad_led_init(5);
    printf("  g_pin = %d (correct, red LED's pin)\n\n", bad_led_get_pin());

    printf("--- Init green LED (pin=3) ---\n");
    bad_led_init(3);
    printf("  g_pin = %d (overwritten!)\n\n", bad_led_get_pin());

    printf("--- Try to turn ON \"red LED\" ---\n");
    printf("  You think you are operating red LED (pin=5)...\n");
    bad_led_on();
    printf("  Actually operating Pin %d (green LED's pin!)\n", bad_led_get_pin());
    printf("\n");
    printf("  [BUG] red LED's pin was overwritten by green LED's init!\n");
    printf("  [CAUSE] g_pin is a global variable, two LEDs share the same data.\n");
    printf("  [RESULT] You thought you controlled red LED, but actually controlled green LED.\n");

    /* ==============================================================
     *  第二部分：正面教材 —— 数据归位后的世界
     *
     *  改造后：pin和brightness存在struct里，跟着me指针走。
     *  红灯有红灯的pin，绿灯有绿灯的pin，互不干扰。
     * ============================================================== */
    printf("\n\n=== Part 2: Good Example (The World After Data Ownership) ===\n\n");

    Led_t red_led;     /* 红灯对象 —— 自己的pin、自己的brightness */
    Led_t green_led;   /* 绿灯对象 —— 自己的pin、自己的brightness */

    printf("--- Init red LED (pin=5) ---\n");
    led_init(&red_led, 5);

    printf("\n--- Init green LED (pin=3) ---\n");
    led_init(&green_led, 3);

    printf("\n--- Check: is red LED's pin still intact? ---\n");
    bool is_on;
    uint8_t brightness;
    led_get_state(&red_led, &is_on, &brightness);
    printf("  red LED pin=%d (not overwritten!)\n", red_led.pin);
    printf("  green LED pin=%d\n", green_led.pin);
    printf("  Two LEDs are independent, no interference.\n");

    printf("\n--- Operate two LEDs separately ---\n");
    printf("[red LED]\n");
    led_on(&red_led);
    led_set_brightness(&red_led, 80);

    printf("[green LED]\n");
    led_on(&green_led);
    led_set_brightness(&green_led, 40);

    printf("\n--- Verify: two LEDs have independent states ---\n");
    bool red_on, green_on;
    uint8_t red_bright, green_bright;
    led_get_state(&red_led, &red_on, &red_bright);
    led_get_state(&green_led, &green_on, &green_bright);
    printf("  red LED:   pin=%d, state=%s, brightness=%d%%\n",
           red_led.pin, red_on ? "ON" : "OFF", red_bright);
    printf("  green LED: pin=%d, state=%s, brightness=%d%%\n",
           green_led.pin, green_on ? "ON" : "OFF", green_bright);

    /* ==============================================================
     *  第三部分：模块级数据 —— static变量 + 函数访问
     *
     *  s_init_count是static变量，外部不能直接访问。
     *  想知道init了几次？调 led_get_init_count()。
     *  数据的主人说了算 —— 这就是封装。
     * ============================================================== */
    printf("\n\n=== Part 3: Module-Level Data (The Power of static) ===\n\n");

    printf("--- Module-level data: init count ---\n");
    printf("  Total init count: %d (red LED + green LED, 2 inits total)\n",
           led_get_init_count());
    printf("  This data is stored in a static variable, cannot be modified externally.\n");
    printf("  Want to check? Call led_get_init_count(). The data owner decides.\n");

    /*
     * 如果你尝试在main.c里这样写：
     *   extern int s_init_count;  // 编译报错！static变量外部不可见
     *
     * 但led_bad.c里的init_count没加static：
     *   extern int init_count;    // 能编译通过！谁都能改！
     *
     * 这就是static的意义：给数据上锁，只有主人能操作。
     */

    /* ==============================================================
     *  收尾：析构
     * ============================================================== */
    printf("\n--- Cleanup: deinit ---\n");
    led_deinit(&red_led);
    led_deinit(&green_led);

    /* ==============================================================
     *  总结：数据归位表
     * ============================================================== */
    printf("\n========================================\n");
    printf("  Summary: Every piece of data has an owner\n");
    printf("  \n");
    printf("  Data Ownership in 3 steps:\n");
    printf("  \n");
    printf("  Before (global)          After (Data Ownership)\n");
    printf("  -------------------------------------\n");
    printf("  int g_pin;        -> me->pin (struct member)\n");
    printf("  int g_brightness; -> me->brightness (struct member)\n");
    printf("  int init_count;   -> static int s_init_count\n");
    printf("  int g_debug_flag; -> static int s_debug_flag\n");
    printf("  int MAX_BRIGHTNESS-> static const uint8_t\n");
    printf("  \n");
    printf("  Three meanings of static:\n");
    printf("    1. Before function = file-private function\n");
    printf("    2. Before global var = file-private variable\n");
    printf("    3. Before local var = persists across calls\n");
    printf("  \n");
    printf("  No owner for data, bug is the owner.\n");
    printf("========================================\n");

    printf("\nPress Enter to exit...\n");
    getchar();
    return 0;
}
