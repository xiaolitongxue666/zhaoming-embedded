/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    pin_board_pc.c
  * @brief   PC mock board-level PIN driver implementation.
  *
  * 在 PC 上用 printf 假装 GPIO 操作. 启动期通过 INIT_BOARD_EXPORT 自动注册
  * 到 platform_pin 框架. 应用层 / driver 层一字不改, 跟 libgpiod / sysfs
  * 子类完全等价的接口.
  *
  * 字符串 pin 名约定跟 libgpiod / sysfs 一致: "GPIO17", "GPIO27"... (BCM 编号).
  ******************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform/platform_pin.h"
#include "platform/platform_module_export.h"

static int32_t _pc_pin_get(const char *name)
{
    if ((name[0] != 'G') || (name[1] != 'P') || (name[2] != 'I') ||
        (name[3] != 'O')) {
        return PLATFORM_EINVAL;
    }
    int n = atoi(name + 4);
    if ((n < 0) || (n > 53)) {
        return PLATFORM_EINVAL;
    }
    return n;
}

static void _pc_pin_mode(int32_t pin, int32_t mode)
{
    const char *m = (mode == PIN_MODE_OUTPUT) ? "OUTPUT" :
                    (mode == PIN_MODE_INPUT)  ? "INPUT"  : "OTHER";
    printf("    [PC] GPIO%d mode -> %s\n", (int)pin, m);
}

static void _pc_pin_write(int32_t pin, int32_t value)
{
    printf("    [PC] GPIO%d <- %s\n", (int)pin,
           value ? "HIGH" : "LOW");
}

static int32_t _pc_pin_read(int32_t pin)
{
    (void)pin;
    return PIN_LOW;
}

static const platform_pin_ops_t _pc_pin_ops =
{
    .pin_mode       = _pc_pin_mode,
    .pin_write      = _pc_pin_write,
    .pin_read       = _pc_pin_read,
    .pin_attach_irq = NULL,
    .pin_detach_irq = NULL,
    .pin_irq_enable = NULL,
    .pin_get        = _pc_pin_get,
};

static void _platform_hw_pin_init(void)
{
    platform_pin_register(&_pc_pin_ops);
}
INIT_BOARD_EXPORT(_platform_hw_pin_init);

/******************** END OF FILE ********************/
