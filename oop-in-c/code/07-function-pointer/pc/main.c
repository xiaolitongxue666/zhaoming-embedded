/* SPDX-License-Identifier: MIT */
/*
 * main.c - 函数指针变量演示
 *
 * 本章核心 (见 ch07 § 7.4):
 *   一个变量 fp 能存"接受 int、返回 void"的函数地址.
 *   函数名不带括号 = 取地址 (存号码)
 *   fp(15)             = 通过变量调用 (拨号)
 *   换一个号码存进去, 同一行 fp(15) 拨通的就是另一个函数.
 *
 * 本章不引入 LED 结构, 不引入 platform 抽象, gpio_on / pwm_on /
 * i2c_on 只是 printf 占位, 用来让 fp 能跳到不同地址跑出不同输出.
 * 真实硬件实现 (HAL_GPIO_WritePin / sysfs write) 见 stm32-snippet
 * 和 linux-snippet, 对函数指针主线本身没有影响.
 *
 * 所有函数签名都是 void (*)(int) 一致 - 这样同一个 fp 能存任何一个,
 * 这是函数指针的硬约束 (类型不一致编译报错). 详见 ch07 § 7.3.
 */

#include <stdio.h>

/* ---- 三种 LED 的 on / off 占位实现 ---- */

static void gpio_on(int pin)
{
	printf("  [GPIO] pin %d ON\n", pin);
}

static void gpio_off(int pin)
{
	printf("  [GPIO] pin %d OFF\n", pin);
}

static void pwm_on(int channel)
{
	printf("  [PWM] channel %d ON (duty 100)\n", channel);
}

static void pwm_off(int channel)
{
	printf("  [PWM] channel %d OFF\n", channel);
}

static void i2c_on(int addr)
{
	printf("  [I2C] addr 0x%02X ON (cmd 0x01)\n", addr);
}

static void i2c_off(int addr)
{
	printf("  [I2C] addr 0x%02X OFF (cmd 0x00)\n", addr);
}

/* ---- 主流程: 一个 fp, 三种号码, 三种行为 ---- */

int main(void)
{
	void (*fp)(int);   /* 一个变量, 能存"接受 int、返回 void"的函数地址 */

	printf("========================================\n");
	printf("  Function pointer = a variable holding code address.\n");
	printf("  Same fp, different number, different call.\n");
	printf("========================================\n\n");

	/*
	 * 存号码: 函数名不带括号 = 取地址.
	 * 拨号:   fp(15) = 通过变量调用, 实际跳到 gpio_on 那段机器码.
	 */
	printf("--- fp = gpio_on; fp(15); ---\n");
	fp = gpio_on;
	fp(15);

	printf("\n--- fp = pwm_on; fp(15); ---\n");
	fp = pwm_on;
	fp(15);

	printf("\n--- fp = i2c_on; fp(0x50); ---\n");
	fp = i2c_on;
	fp(0x50);

	/* off 系列同一个套路, 同一个 fp, 换三次号码. */
	printf("\n--- swap to off-functions ---\n");
	fp = gpio_off;
	fp(15);
	fp = pwm_off;
	fp(15);
	fp = i2c_off;
	fp(0x50);

	printf("\n========================================\n");
	printf("  Same variable fp.\n");
	printf("  Three numbers, three behaviors.\n");
	printf("========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
