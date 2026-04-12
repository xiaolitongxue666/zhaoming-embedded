/**
 * @file     main.c
 * @brief    EP10演示 - HAL库几千个函数，就一个套路
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP10 - HAL库几千个函数，就一个套路
 *
 * 这一期是"验证课"——不教新概念。
 *
 * EP06-09学的所有东西，在HAL库里都有一一对应：
 *
 *   你学的              HAL库里的
 *   ─────────────────  ──────────────────────
 *   Led_t（struct）     GPIO_TypeDef（struct）       ← EP06
 *   red_led/green_led  GPIOA/GPIOB/GPIOC           ← EP06
 *   Led_t *me          GPIO_TypeDef *GPIOx          ← EP06
 *   led_ 前缀          HAL_GPIO_ 前缀               ← EP08
 *   led_init/deinit    HAL_GPIO_Init/DeInit         ← EP08
 *   .h声明 + .c的static  菜单 + 后厨               ← EP07
 *
 * 几千个函数——就这一招。
 */

#include <stdio.h>
#include "hal_gpio.h"

int main(void) {
    printf("============================================================\n");
    printf("  EP10  Thousands of HAL functions, one single pattern\n");
    printf("  \"What you learned IS the industry standard.\"\n");
    printf("============================================================\n\n");

    /* ==============================================================
     *  第一部分：struct封装数据 —— EP06
     *
     *  你学的：Led_t 把 pin/brightness/is_on 打包在一起
     *  HAL库的：GPIO_TypeDef 把 MODER/OTYPER/ODR... 打包在一起
     *
     *  不同的数据，同一个思想——struct封装。
     * ============================================================== */
    printf("=== Part 1: struct Encapsulation (EP06) ===\n\n");

    printf("  GPIO_TypeDef contains these registers:\n");
    printf("    MODER / OTYPER / OSPEEDR / PUPDR / IDR / ODR / BSRR / LCKR\n");
    printf("  Same idea as Led_t containing pin / brightness / is_on.\n");
    printf("  struct bundles related data together -- taught in EP06.\n\n");

    /* ==============================================================
     *  第二部分：多实例 —— EP06
     *
     *  你学的：Led_t red_led; Led_t green_led; → 两颗LED
     *  HAL库的：GPIOA / GPIOB / GPIOC → 三个端口
     *
     *  同一个struct定义，不同的数据——多实例。
     * ============================================================== */
    printf("=== Part 2: Multiple Instances (EP06) ===\n\n");

    printf("  Three GPIO ports, each is GPIO_TypeDef:\n");
    printf("    GPIOA = &g_gpioa_regs  (Port A registers)\n");
    printf("    GPIOB = &g_gpiob_regs  (Port B registers)\n");
    printf("    GPIOC = &g_gpioc_regs  (Port C registers)\n");
    printf("  Same idea as red_led / green_led --\n");
    printf("  Same code, different data.\n\n");

    /* ==============================================================
     *  第三部分：构造函数 + me指针 —— EP06 & EP08
     *
     *  你学的：led_init(Led_t *me, uint8_t pin)
     *  HAL库的：HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *init)
     *
     *  第一个参数都是指向"自己"的指针——me指针。
     *  GPIOx 就是 me！
     * ============================================================== */
    printf("=== Part 3: Constructor + me pointer (EP06 & EP08) ===\n\n");

    /* 准备初始化配置——和传参给led_init是一个道理 */
    GPIO_InitTypeDef gpio_config;

    /* --- GPIOA Pin5：输出模式，推挽，高速 --- */
    printf("--- Init GPIOA Pin5 (LED pin) ---\n");
    gpio_config.Pin   = GPIO_PIN_5;
    gpio_config.Mode  = GPIO_MODE_OUTPUT;
    gpio_config.OType = GPIO_OTYPE_PP;
    gpio_config.Speed = GPIO_SPEED_HIGH;
    gpio_config.Pull  = GPIO_PUPD_NONE;

    /*
     * HAL_GPIO_Init(GPIOA, &gpio_config);
     *              ↑ me指针      ↑ 配置
     *
     * led_init(&red_led, pin);
     *          ↑ me指针   ↑ 配置
     *
     * 一模一样的套路。
     */
    HAL_GPIO_Init(GPIOA, &gpio_config);

    /* --- GPIOB Pin0：输出模式，开漏，中速 --- */
    printf("\n--- Init GPIOB Pin0 (another LED pin) ---\n");
    gpio_config.Pin   = GPIO_PIN_0;
    gpio_config.Mode  = GPIO_MODE_OUTPUT;
    gpio_config.OType = GPIO_OTYPE_OD;
    gpio_config.Speed = GPIO_SPEED_MEDIUM;
    gpio_config.Pull  = GPIO_PUPD_UP;
    HAL_GPIO_Init(GPIOB, &gpio_config);

    /* --- GPIOC Pin13：输出模式（STM32板载LED经典引脚）--- */
    printf("\n--- Init GPIOC Pin13 (classic onboard LED pin) ---\n");
    gpio_config.Pin   = GPIO_PIN_13;
    gpio_config.Mode  = GPIO_MODE_OUTPUT;
    gpio_config.OType = GPIO_OTYPE_PP;
    gpio_config.Speed = GPIO_SPEED_LOW;
    gpio_config.Pull  = GPIO_PUPD_NONE;
    HAL_GPIO_Init(GPIOC, &gpio_config);

    printf("\n  Same HAL_GPIO_Init function, operated on three different ports.\n");
    printf("  The first parameter GPIOx decides WHO to operate on -- that's the me pointer.\n\n");

    /* ==============================================================
     *  第四部分：操作函数 —— EP06的me指针实战
     *
     *  你学的：led_on(&red_led)  / led_on(&green_led)
     *  HAL库的：HAL_GPIO_WritePin(GPIOA, ...) / HAL_GPIO_WritePin(GPIOB, ...)
     *
     *  同一个函数，不同的me指针——操作不同的实例。
     * ============================================================== */
    printf("=== Part 4: Same function, different me pointers (EP06) ===\n\n");

    printf("--- WritePin: Write pin level ---\n");
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, true);     /* GPIOA Pin5 高电平 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, true);     /* GPIOB Pin0 高电平 */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, false);   /* GPIOC Pin13 低电平 */

    printf("\n--- ReadPin: Read pin level ---\n");
    HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);
    HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
    HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

    printf("\n--- TogglePin: Toggle pin (same pattern as led_toggle) ---\n");
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);   /* 高→低 */
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);  /* 低→高 */

    printf("\n--- Read again to verify toggle results ---\n");
    HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);
    HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

    /* ==============================================================
     *  第五部分：后厨的static函数 —— EP07
     *
     *  hal_gpio.c 里有这些static函数：
     *    static get_pin_number()   —— 引脚位掩码转编号
     *    static set_2bit_field()   —— 设置2-bit寄存器字段
     *    static set_1bit_field()   —— 设置1-bit寄存器字段
     *    static mode_name()        —— 模式转中文名
     *
     *  你在hal_gpio.h里看不到它们——因为那是后厨的事。
     *  和EP07的 static update_hardware() 是一个道理。
     * ============================================================== */
    printf("\n=== Part 5: Kitchen-only static functions (EP07) ===\n\n");

    printf("  hal_gpio.c has these static functions (kitchen-only):\n");
    printf("    static get_pin_number()  -- pin bitmask to pin number\n");
    printf("    static set_2bit_field()  -- set 2-bit register field\n");
    printf("    static set_1bit_field()  -- set 1-bit register field\n");
    printf("    static mode_name()       -- mode to string\n");
    printf("\n");
    printf("  You can't see them in hal_gpio.h.\n");
    printf("  .h = menu (what customers can see)\n");
    printf("  static in .c = kitchen (hidden from customers)\n");
    printf("  Taught in EP07.\n\n");

    /* ==============================================================
     *  第六部分：析构函数 —— EP08
     *
     *  你学的：led_deinit(&red_led)
     *  HAL库的：HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5)
     *
     *  恢复默认状态——打烊收工。
     * ============================================================== */
    printf("=== Part 6: Destructor = closed for the day (EP08) ===\n\n");

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_13);

    /* ==============================================================
     *  总结：映射对照表
     * ============================================================== */
    printf("\n============================================================\n");
    printf("  EP10 Summary: What you learned IS the industry standard\n");
    printf("============================================================\n\n");

    printf("  What you learned       What HAL uses\n");
    printf("  ----------------------  -------------------------\n");
    printf("  Led_t (struct)        GPIO_TypeDef (struct)        [EP06 Encapsulation]\n");
    printf("  red_led / green_led   GPIOA / GPIOB / GPIOC       [EP06 Multi-instance]\n");
    printf("  Led_t *me             GPIO_TypeDef *GPIOx          [EP06 me pointer]\n");
    printf("  led_ prefix           HAL_GPIO_ prefix             [EP08 Class name]\n");
    printf("  led_init / deinit     HAL_GPIO_Init / DeInit       [EP08 Ctor/Dtor]\n");
    printf("  .h decl + static in .c  menu + kitchen             [EP07 Info hiding]\n");
    printf("\n");
    printf("  Thousands of functions -- one single pattern.\n");
    printf("============================================================\n");

    printf("\nPress Enter to exit...\n");
    getchar();
    return 0;
}
