/* SPDX-License-Identifier: MIT */
/**
 * @file    platform_pc.c
 * @brief   平台抽象层 - PC 模拟实现（platform.h 的 PC 版本）
 * @note    在 PC 上用 printf 模拟 GPIO 动作，没有开发板也能编译运行，
 *          看到 LED 框架的"逻辑效果"。
 *
 *          这一份是全书章节共用的最朴素实现：四个封装函数直接落到
 *          printf 上。STM32 / Linux 平台有各自的真实硬件实现版本，
 *          对外签名跟这一份完全一样，所以驱动层代码不需要任何改动。
 */

#include "platform.h"
#include <stdio.h>

void platform_gpio_init(uint8_t pin, uint8_t mode)
{
    const char *mode_str = (mode == GPIO_MODE_OUTPUT) ? "OUTPUT" : "INPUT";
    printf("[GPIO] Pin%d init as %s\n", pin, mode_str);
}

void platform_gpio_deinit(uint8_t pin)
{
    printf("[GPIO] Pin%d released\n", pin);
}

void platform_gpio_write(uint8_t pin, bool value)
{
    printf("[GPIO] Pin%d -> %s\n", pin, value ? "HIGH (ON)" : "LOW (OFF)");
}

bool platform_gpio_read(uint8_t pin)
{
    printf("[GPIO] Read Pin%d -> LOW\n", pin);
    return false;
}
