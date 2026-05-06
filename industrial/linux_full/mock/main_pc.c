/**
  ******************************************************************************
  * @file    main_pc.c
  * @brief   PC mock entry, replaces src/main.c on host PC builds.
  *
  * 验证 linux_full 整套抽象在 PC 上能 build + run, 不依赖 libgpiod 或 sysfs.
  * 所有 GPIO 操作走 pin_board_pc.c 打到 stdout.
  ******************************************************************************
  */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "led/led_base.h"
#include "platform/platform_module_export.h"

extern led_base_t *led_green;
extern led_base_t *led_orange;
extern led_base_t *led_red;
extern led_base_t *led_blue;

#define ROUNDS  3

int main(void)
{
    led_base_t *all[4];
    uint32_t cur;
    uint32_t round;

    printf("=========================================\n");
    printf("  linux_full PC mock: 4 LED running light\n");
    printf("=========================================\n");

    platform_module_export_exec();

    if ((NULL == led_green)  || (NULL == led_orange) ||
        (NULL == led_red)    || (NULL == led_blue)) {
        printf("[main] LED handles not ready, exit.\n");
        return -1;
    }

    all[0] = led_green;
    all[1] = led_orange;
    all[2] = led_red;
    all[3] = led_blue;

    cur = 0;
    for (round = 0; round < ROUNDS * 4; round++) {
        printf("\n--- step %u ---\n", (unsigned)round);
        for (uint32_t i = 0; i < 4; i++) {
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
