/* SPDX-License-Identifier: MIT */
/*
 * main.c - 函数指针当参数演示
 *
 * 同一个 test_led, 三种 LED.
 *   test_led(gpio_on, gpio_off, 15)    -> GPIO 引脚 15
 *   test_led(pwm_on,  pwm_off,  3)     -> PWM 通道 3
 *   test_led(i2c_on,  i2c_off,  0x50)  -> I2C 地址 0x50
 *
 * test_led 函数体写完之后再没动过, 决定"拨给谁"的是调用现场传进去的
 * 那两个函数指针. 这是函数指针当参数最直接的形态: 把"调谁"留到调用
 * 时再说, 而不是在写函数体时就定死.
 */

#include <stdio.h>
#include "led.h"

int main(void)
{
	printf("========================================\n");
	printf("  Function pointer as a parameter.\n");
	printf("  Three LEDs, one test_led.\n");
	printf("========================================\n\n");

	printf("--- test_led(gpio_on, gpio_off, 15) ---\n");
	test_led(gpio_on, gpio_off, 15);

	printf("\n--- test_led(pwm_on, pwm_off, 3) ---\n");
	test_led(pwm_on, pwm_off, 3);

	printf("\n--- test_led(i2c_on, i2c_off, 0x50) ---\n");
	test_led(i2c_on, i2c_off, 0x50);

	printf("\n========================================\n");
	printf("  Same test_led, three different LEDs.\n");
	printf("  Late binding -- decide at runtime.\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
