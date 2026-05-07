/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    main_pc.c
  * @brief   PC mock entry, replaces Core/Src/main.c on host PC builds.
  *
  * 见附录 B § B.6 "PC mock 双模 build".
  *
  * 验证 stm32_full 整套抽象在 PC 上能 build + run. 所有 GPIO 操作走
  * pin_board_pc.c 打到 stdout, 没有真机时也能看流水灯逻辑. 这种"真机
  * + PC mock 双模 build"是工业项目里常见做法, 让 OOP 抽象在 PC 上也能
  * 单元测试 / 跑 CI.
  *
  * 真机 main 调 platform_module_export_exec() 跑完所有 INIT_xxx_EXPORT;
  * MOCK 模式下 ctor 已经在 main 之前自动跑过, 这里调 platform_module_export_exec
  * 是 nop. 应用层调用形态完全一致.
  ******************************************************************************
  */

#include <stdio.h>
#include <stdint.h>

#include "led/led_base.h"
#include "platform/platform_module_export.h"

/* 板子上 4 颗 LED 句柄 (定义在 environment_cfg/led_cfg.c) */
extern led_base_t *led_green;
extern led_base_t *led_orange;
extern led_base_t *led_red;
extern led_base_t *led_blue;

#define ROUNDS  3   /* 流水灯跑 3 圈就退出 */

int main(void)
{
    led_base_t *all[4];
    uint32_t cur;
    uint32_t round;

    printf("=========================================\n");
    printf("  stm32_full PC mock: 4 LED running light\n");
    printf("=========================================\n");

    /* 真机这一行跑 7 级 INIT_xxx_EXPORT;
     * MOCK 下 ctor 已经在 main 之前跑过, 这里是 nop. */
    platform_module_export_exec();

    if (NULL == led_green || NULL == led_orange ||
        NULL == led_red   || NULL == led_blue)
    {
        printf("[main] LED handles not ready, exit.\n");
        return -1;
    }

    all[0] = led_green;
    all[1] = led_orange;
    all[2] = led_red;
    all[3] = led_blue;

    cur = 0;
    for (round = 0; round < ROUNDS * 4; round++)
    {
        printf("\n--- step %u ---\n", (unsigned)round);
        for (uint32_t i = 0; i < 4; i++)
        {
            led_off(all[i]);
        }
        led_on(all[cur]);
        cur = (cur + 1) % 4;
    }

    printf("\n=========================================\n");
    printf("  done (%u rounds)\n", (unsigned)ROUNDS);
    printf("=========================================\n");
    return 0;
}

/******************** END OF FILE ********************/
