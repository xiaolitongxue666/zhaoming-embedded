/* SPDX-License-Identifier: MIT */
/**
 * @file  led.c
 * @brief 三种 LED 的 on/off 占位实现 + test_led
 *
 * @details
 * 六个 on/off 都是 printf 占位, 用来让 test_led 内部"拨号"时跑出
 * 不同输出. 真实硬件操作完整工程见 ch15 15-platform/, 主线
 * 不依赖具体硬件.
 *
 * test_led 自己不知道调谁, 调用方告诉它. 函数指针的本质是延迟决定,
 * 不是现在就定死, 而是到时候再说.
 *
 * NULL check 不能省: 函数指针参数为 NULL 时调过去等于跳到地址 0,
 * 在 STM32 上就是 HardFault. 工业代码里所有函数指针调用前都 NULL check.
 */

#include "led.h"
#include <stdio.h>

/* ---- 三种 LED 的 on / off 占位实现 ---- */

void gpio_on(int pin)
{
	printf("  [GPIO] pin %d ON\n", pin);
}

void gpio_off(int pin)
{
	printf("  [GPIO] pin %d OFF\n", pin);
}

void pwm_on(int channel)
{
	printf("  [PWM] channel %d ON (duty 100)\n", channel);
}

void pwm_off(int channel)
{
	printf("  [PWM] channel %d OFF\n", channel);
}

void i2c_on(int addr)
{
	printf("  [I2C] addr 0x%02X ON (cmd 0x01)\n", addr);
}

void i2c_off(int addr)
{
	printf("  [I2C] addr 0x%02X OFF\n", addr);
}

/*
 * 通用测试工具函数.
 *
 * test_led 接受两个函数指针 + 一个 id. 函数体不知道也不需要知道这两个
 * 函数具体是哪个文件、哪个实现 -- 它信任调用方传进来的是合法的 on/off,
 * 按 "open -> hold -> close" 三步跑完.
 */
void test_led(void (*on)(int), void (*off)(int), int id)
{
	if (!on || !off)
		return;

	printf("  [test] open ...\n");
	on(id);
	printf("  [test] hold ...\n");
	printf("  [test] close ...\n");
	off(id);
}
