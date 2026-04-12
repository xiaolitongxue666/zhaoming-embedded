/**
 * @file     main.c
 * @brief    EP08演示 - 两个"类"和平共处，前缀就是类名
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP08 - 你用C手搓了一个class
 *
 * 这个程序演示的核心问题和解决方案：
 *
 *   问题1：LED模块和电机模块都有 init/on/off → 编译报错"名称冲突"
 *   解决1：函数前缀 = 类名（led_init vs motor_init，永远不冲突）
 *
 *   问题2：实习生不知道调用顺序，没调init直接用 → 系统崩溃
 *   解决2：init = 构造函数（开门营业），deinit = 析构函数（打烊收工）
 *
 *   一个 .h + 一个 .c = 一个完整的C语言"类"
 *
 *   金句：C语言没有class？你天天都在写。
 */

#include <stdio.h>
#include "led.h"
#include "motor.h"

int main(void)
{
    int ret;

    printf("========================================\n");
    printf("  EP08 You just built a class in C\n");
    printf("  \"No class in C? You write one every day.\"\n");
    printf("========================================\n\n");

    /* ==============================================================
     *  第一部分：两个"类"和平共处
     *  led_init 和 motor_init 前缀不同，永远不冲突
     * ============================================================== */
    printf("=== Part 1: Two modules, different prefixes, no conflict ===\n\n");

    Led_t   red_led;    /* LED对象 - 引脚5  */
    Motor_t fan_motor;  /* 电机对象 - 引脚8 */

    /*
     * 如果没有前缀，两个模块都叫 init()，编译器直接报错：
     *   "multiple definition of `init`"
     *
     * 有了前缀：
     *   led_init()   —— LED的构造函数
     *   motor_init() —— 电机的构造函数
     *   名字不同，和平共处。
     *
     * 前缀就像品牌名："沙县小吃"和"兰州拉面"都卖汤，
     * 但你不会走错门。
     */
    printf("--- Constructor: init = open for business ---\n");
    printf("[LED class]\n");
    led_init(&red_led, 5);

    printf("[Motor class]\n");
    motor_init(&fan_motor, 8);

    /* ==============================================================
     *  第二部分：正常使用——init之后才能操作
     * ============================================================== */
    printf("\n=== Part 2: Normal usage (init first, then operate) ===\n\n");

    printf("--- LED operations ---\n");
    led_on(&red_led);
    led_set_brightness(&red_led, 75);

    bool led_state;
    uint8_t led_bright;
    led_get_state(&red_led, &led_state, &led_bright);
    printf("  [Query] LED state: %s, brightness: %d%%\n", led_state ? "ON" : "OFF", led_bright);

    printf("\n--- Motor operations ---\n");
    motor_set_speed(&fan_motor, 60);
    motor_start(&fan_motor);

    bool motor_running;
    uint8_t motor_speed;
    motor_get_state(&fan_motor, &motor_running, &motor_speed);
    printf("  [Query] Motor state: %s, speed: %d%%\n",
           motor_running ? "running" : "stopped", motor_speed);

    /* ==============================================================
     *  第三部分：没调init直接用的后果
     *  实习生的经典操作——系统崩溃的根源
     * ============================================================== */
    printf("\n=== Part 3: Using without init? (classic intern mistake) ===\n\n");

    /*
     * 场景还原：实习生拿到代码，不看文档，上来就调 motor_start()。
     * 如果没有 initialized 检查，GPIO没配置就写电平 → 硬件行为不可预测。
     *
     * 有了 init 里的 initialized 标志位：
     *   - 没初始化就使用 → 函数立刻返回错误码
     *   - 不会操作未配置的硬件
     *   - 程序不崩溃，还告诉你哪里错了
     *
     * 这就是构造函数的意义：确保对象在使用前处于合法状态。
     * 没开门营业，你不能进店吃饭。
     */
    Motor_t new_motor = {0};   /* 只声明，没调 motor_init */

    printf("--- Calling motor_start without motor_init ---\n");
    ret = motor_start(&new_motor);
    printf("  Return: %d (-3 = not initialized)\n", ret);

    printf("\n--- Calling motor_set_speed without motor_init ---\n");
    ret = motor_set_speed(&new_motor, 50);
    printf("  Return: %d (-3 = not initialized)\n", ret);

    Led_t new_led = {0};       /* 只声明，没调 led_init */

    printf("\n--- Calling led_on without led_init ---\n");
    ret = led_on(&new_led);
    printf("  Return: %d (-3 = not initialized)\n", ret);

    /* ==============================================================
     *  第四部分：参数检查——构造函数的第一件事
     * ============================================================== */
    printf("\n=== Part 4: Parameter check (constructor's first line of defense) ===\n\n");

    Led_t bad_led;

    printf("--- Pin out of range (passed 20, valid range 0~15) ---\n");
    ret = led_init(&bad_led, 20);
    printf("  Return: %d (-2 = pin out of range)\n", ret);

    printf("\n--- NULL pointer check ---\n");
    ret = led_init(NULL, 5);
    printf("  Return: %d (-1 = invalid param)\n", ret);

    ret = motor_init(NULL, 8);
    printf("  Return: %d (-1 = invalid param)\n", ret);

    /* ==============================================================
     *  第五部分：析构函数——打烊收工
     *  init第一个调，deinit最后调
     * ============================================================== */
    printf("\n=== Part 5: Destructor = closed ===\n\n");

    /*
     * deinit 做两件事：
     *   1. 关闭硬件（LED灭灯、电机停转）—— 确保安全
     *   2. 释放GPIO资源 —— 归还系统资源
     *
     * 完整的生命周期：
     *   init(构造) → 使用 → deinit(析构)
     *   开门营业   → 服务 → 打烊收工
     *
     * C++ 的析构函数在对象离开作用域时自动调用；
     * C 语言需要手动调 deinit —— 就这点区别。
     */
    printf("--- LED shutdown ---\n");
    led_deinit(&red_led);

    printf("--- Motor shutdown ---\n");
    motor_deinit(&fan_motor);

    /* ==============================================================
     *  总结
     * ============================================================== */
    printf("\n========================================\n");
    printf("  Summary: You just built a class in C!\n");
    printf("  \n");
    printf("  1. led_ prefix = Led class name\n");
    printf("     motor_ prefix = Motor class name\n");
    printf("  2. init = constructor (open for business)\n");
    printf("     deinit = destructor (closed)\n");
    printf("  3. one .h + one .c = a complete class\n");
    printf("  \n");
    printf("  C style       <->  C++ style\n");
    printf("  led_init()    <->  Led::Led()\n");
    printf("  led_on()      <->  Led::on()\n");
    printf("  led_deinit()  <->  Led::~Led()\n");
    printf("  led_ prefix   <->  Led:: scope\n");
    printf("  \n");
    printf("  No class in C? You write one every day.\n");
    printf("========================================\n");

    printf("\nPress Enter to exit...\n");
    getchar();
    return 0;
}
