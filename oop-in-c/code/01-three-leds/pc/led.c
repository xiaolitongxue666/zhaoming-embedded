/* SPDX-License-Identifier: MIT */
/*
 * led.c - LED 模块实现
 *
 * 见书 ch01 § 1.4 函数怎么知道现在是哪张单子。
 *
 * 三颗红绿蓝 LED 共用同一份 led_on / led_off / led_toggle 代码。
 * 函数通过 me 指针知道现在操作的是哪一颗。
 *
 * C++ 里 led.on() 的隐藏 this 指针，在 C 里就是这里手动传的 me。
 * 编译器帮你做的事，你自己手写一遍 —— 这是封装最朴素的形态。
 *
 * 为什么每个函数开头都 if (!me) return -1：见书 § 1.7.2。
 * C 不像 Java 会抛 NullPointerException，对 NULL 解引用在 STM32 上
 * 触发 HardFault，在 Linux 用户态触发 SIGSEGV。工业代码所有公开 API
 * 的指针参数都必须做 NULL 检查，是工程纪律的最小单位。
 */

#include "led.h"
#include <stdio.h>

/*
 * 内部小工具: 把 pin 编码 (高 4 位 port, 低 4 位 num) 拆回 port 字母 + 号,
 * 让日志里看到 "PA.13" 这种 port + 号的可读形态. 这两个函数和
 * common/platform_pc.c 里完全一致, 是教学版图 -- 同样的解码逻辑
 * 在 platform 层有一份, 在 LED 层这里再有一份, 各自只服务自己的 printf.
 */
static char led_pin_port(uint8_t pin)
{
	return (char)('A' + ((pin >> 4) & 0x0F));
}

static int led_pin_num(uint8_t pin)
{
	return pin & 0x0F;
}

int led_init(struct led *me, uint8_t pin)
{
	if (!me)
		return -1;

	me->pin = pin;
	me->brightness = 0;
	me->is_on = false;

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, false);

	printf("  [LED] P%c.%d initialized\n", led_pin_port(pin), led_pin_num(pin));
	return 0;
}

int led_deinit(struct led *me)
{
	if (!me)
		return -1;

	platform_gpio_write(me->pin, false);
	platform_gpio_deinit(me->pin);

	me->is_on = false;
	me->brightness = 0;

	printf("  [LED] P%c.%d released\n", led_pin_port(me->pin), led_pin_num(me->pin));
	return 0;
}

int led_on(struct led *me)
{
	if (!me)
		return -1;

	me->is_on = true;
	platform_gpio_write(me->pin, true);

	printf("  [LED] P%c.%d ON\n", led_pin_port(me->pin), led_pin_num(me->pin));
	return 0;
}

int led_off(struct led *me)
{
	if (!me)
		return -1;

	me->is_on = false;
	platform_gpio_write(me->pin, false);

	printf("  [LED] P%c.%d OFF\n", led_pin_port(me->pin), led_pin_num(me->pin));
	return 0;
}

int led_toggle(struct led *me)
{
	if (!me)
		return -1;

	if (me->is_on)
		led_off(me);
	else
		led_on(me);

	return 0;
}

int led_set_brightness(struct led *me, uint8_t brightness)
{
	if (!me)
		return -1;

	if (brightness > 100) {
		printf("  [LED] Error: brightness %u out of range (0~100)\n",
		       (unsigned)brightness);
		return -2;
	}

	me->brightness = brightness;

	/*
	 * 真实硬件上这里会配 PWM 占空比。PC 模拟环境用 GPIO 高低近似:
	 * 亮度 0 关灯，>0 开灯。等到第 5 章讲 HAL 映射时再细化。
	 */
	if (brightness == 0) {
		me->is_on = false;
		platform_gpio_write(me->pin, false);
	} else {
		me->is_on = true;
		platform_gpio_write(me->pin, true);
	}

	printf("  [LED] P%c.%d brightness set to %u%%\n",
	       led_pin_port(me->pin), led_pin_num(me->pin), (unsigned)brightness);
	return 0;
}
