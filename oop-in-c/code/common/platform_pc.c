/**
 * @file    platform_pc.c
 * @brief   平台抽象层 - PC 模拟实现（platform.h 的 PC 版本）
 * @author  兆鸣嵌入式
 * @note    在 PC 上用 printf 模拟 GPIO 动作，没有开发板也能编译运行，
 *          看到 LED 框架的"逻辑效果"。
 *
 *          这一份是 ch01 - ch14 共用的"最朴素"实现：四个封装函数直接落到
 *          printf 上，没有 ops 表、没有 dispatch、不可运行时切换平台。
 *          ch15 起 platform 层内部演化出 ops 表（platform_ops.h）和分发
 *          (platform_dispatch.c)，可以运行时在 PC / STM32 / Linux 三种
 *          实现间切换；这一份的对外签名跟 ch15 完全一样，所以驱动层
 *          代码从 ch01 到 ch15 不需要任何改动。
 *
 *          这就是"对外签名稳定"的威力：你的应用层 / 驱动层从第一天起就
 *          只依赖这 4 个函数，platform 层内部怎么进化都不会波及上面。
 *          见 ch15 § 15.5 platform 层 ops 化 + ch16 § 16.3 四层架构。
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
