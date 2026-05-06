/**
  ******************************************************************************
  * @file    main.c
  * @brief   STM32F407 firmware entry (CubeMX 风格).
  *
  * 真机入口骨架: 上电 -> startup -> Reset_Handler -> SystemInit -> main ->
  *   HAL_Init -> SystemClock_Config -> MX_GPIO_Init -> 调
  *   platform_module_export_exec() 顺序跑 7 级 INIT_xxx_EXPORT 注册项 ->
  *   主循环.
  *
  * 跟 CubeMX 生成的 main.c 对齐: 函数命名、USER CODE 注释段都按 CubeMX 风
  * 写, 方便用户把这一份覆盖回 CubeMX 工程.
  ******************************************************************************
  */

#include <stdint.h>
#include <stddef.h>

#include "main.h"
#include "led/led_base.h"
#include "platform/platform_module_export.h"

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* extern handles (定义在 app/environment_cfg/led_cfg.c) */
extern led_base_t *led_green;
extern led_base_t *led_orange;
extern led_base_t *led_red;
extern led_base_t *led_blue;

/* CubeMX 生成的函数原型 (用户自己 CubeMX 工程提供) */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/**
  * @brief  Powered-on busy delay, ~1ms scale.
  * @param  ms  milliseconds.
  */
static void delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms * 16800UL; i++)
    {
        ;   /* 168 MHz / 10000, 粗略毫秒级延时 */
    }
}

/**
  * @brief  The application entry point.
  */
int main(void)
{
    led_base_t *all[4];
    uint32_t cur = 0;

    /* MCU Configuration */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();

    /* USER CODE BEGIN 2 */

    /* 这一行调用所有 INIT_BOARD/PREV/DEVICE/COMPONENT/ENV/APP/SYSTEM_READY
     * 注册项: pin_board.c 注册 pin ops, led_cfg.c 装配 4 颗 LED 实例. */
    platform_module_export_exec();

    all[0] = led_green;
    all[1] = led_orange;
    all[2] = led_red;
    all[3] = led_blue;

    /* USER CODE END 2 */

    while (1)
    {
        /* 应用层: 流水灯 */
        for (uint32_t i = 0; i < 4; i++)
        {
            led_off(all[i]);
        }
        led_on(all[cur]);
        cur = (cur + 1) % 4;
        delay_ms(1000);
    }
}

/******************** END OF FILE ********************/
