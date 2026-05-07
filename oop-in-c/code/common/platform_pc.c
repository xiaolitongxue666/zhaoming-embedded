/* SPDX-License-Identifier: MIT */
/*
 * platform_pc.c - platform.h 的 PC 实现.
 *
 * pc/ 不是"伪硬件模拟", 它是 platform 层的一种实现, 和 STM32 / NXP / Linux
 * 平等. STM32 端把 pin 编码翻译成 HAL_GPIO_*, NXP 翻译成自家 SDK,
 * Linux 写 sysfs, 这一份 PC 实现把 pin 编码翻译成 stdout printf.
 * 四份共用同一份 platform.h 头, 应用层 led.c 一字不动.
 *
 * pin 编码和 STM32 端 (oop-in-c/code/01-three-leds/platform-mcu/stm32/
 * led_stm32.c) 字节级一致: 高 4 位 port (A=0, B=1, ..., I=8),
 * 低 4 位 pin 号 (0-15). 这里把它解析回 PA.13 / PD.12 这种人能读的形态
 * 再 printf.
 *
 * 向下兼容: 旧章节里 pin = 13 这种写法 (没用 PIN_NUM 宏) 自动解析为
 * PA.13 (高 4 位 0 = port A, 低 4 位 13 = num 13), 语义恰好对得上.
 */

#include "platform.h"
#include <stdio.h>

/*
 * 把 uint8_t 编码拆回 port 字母 + pin 号.
 * 静态函数, 链接期对外不可见, 是这份文件内部的小工具.
 */
static char pin_port(uint8_t pin)
{
	return (char)('A' + ((pin >> 4) & 0x0F));
}

static int pin_num(uint8_t pin)
{
	return pin & 0x0F;
}

void platform_gpio_init(uint8_t pin, uint8_t mode)
{
	const char *mode_str = (mode == GPIO_MODE_OUTPUT) ? "OUTPUT" : "INPUT";
	printf("[GPIO] P%c.%d init as %s\n", pin_port(pin), pin_num(pin), mode_str);
}

void platform_gpio_deinit(uint8_t pin)
{
	printf("[GPIO] P%c.%d released\n", pin_port(pin), pin_num(pin));
}

void platform_gpio_write(uint8_t pin, bool value)
{
	printf("[GPIO] P%c.%d -> %s\n", pin_port(pin), pin_num(pin),
	       value ? "HIGH (ON)" : "LOW (OFF)");
}

bool platform_gpio_read(uint8_t pin)
{
	printf("[GPIO] Read P%c.%d -> LOW\n", pin_port(pin), pin_num(pin));
	return false;
}
