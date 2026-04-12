/**
 * @file     main.c
 * @brief    EP07演示 - 信息隐藏：正确用法 vs 危险用法
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP07 - 信息隐藏：static与头文件
 *
 * 这个程序演示的核心问题和解决方案：
 *
 *   问题：EP06里struct成员暴露在外，同事直接写了 led.is_on = true，
 *         但忘了操作GPIO —— 软件以为灯亮了，硬件其实没动，LED全乱了。
 *
 *   解决：
 *   1. 头文件只暴露函数声明（菜单），不鼓励直接访问成员
 *   2. .c里用 static 函数隐藏实现细节（后厨）
 *   3. 提供 led_get_state() 安全读取状态
 *
 *   信息隐藏不是不信任 —— 是锁上门，谁都不容易犯错。
 */

#include <stdio.h>
#include "led.h"

int main(void)
{
    printf("========================================\n");
    printf("  EP07 Information Hiding: static & Header Files\n");
    printf("  \"Lock the door, so nobody makes mistakes easily\"\n");
    printf("========================================\n\n");

    Led_t red_led;      /* 红灯 - 引脚13 */
    Led_t green_led;    /* 绿灯 - 引脚14 */

    /* ==============================
     *  第一部分：正确的用法
     *  通过函数接口操作，状态和硬件永远一致
     * ============================== */
    printf("=== Part 1: Correct Usage (via function interface) ===\n\n");

    printf("--- Init ---\n");
    led_init(&red_led, 13);
    led_init(&green_led, 14);

    printf("\n--- Turn on red LED (via led_on function) ---\n");
    led_on(&red_led);

    /* 通过 led_get_state 安全地读取状态 */
    bool state;
    uint8_t bright;
    led_get_state(&red_led, &state, &bright);
    printf("  [Query] Red LED state: %s, brightness: %d%%\n", state ? "ON" : "OFF", bright);

    printf("\n--- Set green LED brightness to 80%% ---\n");
    led_set_brightness(&green_led, 80);

    /* 只查询开关状态，亮度传NULL */
    led_get_state(&green_led, &state, NULL);
    printf("  [Query] Green LED state: %s\n", state ? "ON" : "OFF");

    /* 只查询亮度，开关状态传NULL */
    led_get_state(&green_led, NULL, &bright);
    printf("  [Query] Green LED brightness: %d%%\n", bright);

    printf("\n--- Toggle red LED ---\n");
    led_toggle(&red_led);
    led_get_state(&red_led, &state, NULL);
    printf("  [Query] Red LED state: %s\n", state ? "ON" : "OFF");

    /* ==============================
     *  第二部分：危险的用法（反面教材）
     *  直接修改struct成员，后果是什么？
     * ============================== */
    printf("\n=== Part 2: Dangerous Usage (modify struct directly) ===\n");
    printf("=== Below is a BAD example, showing why Information Hiding is needed ===\n\n");

    printf("--- Re-init red LED for demo ---\n");
    led_init(&red_led, 13);

    /*
     * 危险操作！直接修改 struct 成员。
     *
     * 同事心想："我把 is_on 改成 true，灯不就亮了吗？"
     * 然而他忘了调 GPIO —— 软件记录说灯亮着，实际引脚没动。
     * 这就是"状态和硬件不一致"的bug。
     *
     * 如果用 led_on() 函数，函数内部会同时：
     *   1. 更新 is_on = true
     *   2. 调用 update_hardware() 操作GPIO
     * 两步绑在一起，不可能漏掉。
     */
    printf("--- Directly write red_led.is_on = true (Danger!) ---\n");
    red_led.is_on = true;  /* 只改了软件状态... */
    /* 忘了操作GPIO！硬件根本没变！ */

    printf("  [Danger] Software thinks red LED is ON, but GPIO was never touched!\n");
    printf("  [Danger] On real hardware, the LED is still OFF.\n");

    /*
     * 再来一个危险操作：直接改亮度
     * 如果是通过 led_set_brightness() 函数，函数会：
     *   1. 检查亮度范围（0~100）
     *   2. 更新 brightness 和 is_on
     *   3. 调用 update_hardware() 操作GPIO
     * 直接改成员，这三步全跳过了。
     */
    printf("\n--- Directly write red_led.brightness = 200 (Danger!) ---\n");
    red_led.brightness = 200;  /* 没有范围检查！200超出0~100！ */
    printf("  [Danger] Brightness set to 200, out of valid range, no check!\n");
    printf("  [Danger] On real hardware, PWM may overflow, behavior undefined.\n");

    /*
     * 对比：正确的方式
     * led_set_brightness 会检查范围并返回错误码
     */
    printf("\n--- Compare: set brightness 200 via function (Safe!) ---\n");
    int ret = led_set_brightness(&red_led, 200);
    printf("  [Safe] Function returned error code %d, brightness not set to illegal value\n", ret);

    /*
     * static函数的效果：
     *
     * led.c 里有一个 static void update_hardware(Led_t *me)，
     * 它负责所有的GPIO操作。
     *
     * 你在 main.c 里试试调用 update_hardware(&red_led) ——
     * 编译器会报错："undefined reference"
     * 因为 static 函数只在 led.c 内部可见，外面调不到。
     *
     * 这就是C语言的 private：
     *   static函数 = private方法（后厨，顾客进不去）
     *   头文件里的函数声明 = public方法（菜单，顾客能点）
     */
    printf("\n--- Effect of static ---\n");
    printf("  update_hardware() in led.c is a static function,\n");
    printf("  cannot be called from main.c -- this is C's private.\n");
    printf("  Try uncommenting the line below, compiler will report error:\n");
    printf("  // update_hardware(&red_led);  /* compile error! */\n");

    /* ==============================
     *  第三部分：清理
     * ============================== */
    printf("\n=== Part 3: Cleanup ===\n\n");
    led_deinit(&red_led);
    led_deinit(&green_led);

    printf("\n========================================\n");
    printf("  Summary:\n");
    printf("  - Header file = menu (only function declarations)\n");
    printf("  - static function = private (back kitchen, outsiders can't enter)\n");
    printf("  - led_get_state() = safe way to read state\n");
    printf("  - Information Hiding is not distrust, it's locking the door,\n");
    printf("    so nobody makes mistakes easily.\n");
    printf("========================================\n");

    printf("\nPress Enter to exit...\n");
    getchar();
    return 0;
}
