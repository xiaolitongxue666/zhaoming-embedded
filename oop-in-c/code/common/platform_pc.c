/**
 * @file    platform_pc.c
 * @brief   平台抽象层 - PC模拟实现
 * @author  兆鸣嵌入式
 * @note    在PC上用printf模拟GPIO操作。
 *          没有开发板也能编译运行，看到LED的"逻辑效果"。
 *
 *          如果你有STM32开发板，可以写一个 platform_stm32.c，
 *          实现同样的接口，上层代码一行不改——这就是平台抽象的威力。
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
