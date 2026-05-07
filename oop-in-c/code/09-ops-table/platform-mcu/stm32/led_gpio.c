/* SPDX-License-Identifier: MIT */
/*
 * led_gpio.c - LED GPIO 子类 STM32 端真机实现
 *
 * 这是子类内部的 STM32 真机版本 (片段, 不是完整工程). 完整 STM32 工程
 * 见附录 B (industrial/stm32_full).
 *
 * 跟 pc/ 唯一的差别就在这个文件: pc/ 用 printf 模拟硬件动作, STM32 上
 * gpio_xxx 走 HAL_GPIO_WritePin. ops 表的形态、子类 init 的形态、父类
 * 走 ops 分发的形态全部一字不动.
 *
 * pin 编码 (uint8_t 同时表示 port + pin 号) 跟 ch01 platform-mcu/stm32/
 * led_stm32.c 字节级一致, 见 oop-in-c/code/common/platform.h 的 PIN_NUM 宏.
 *
 * 见 ch09 § 9.5 在 STM32 上长什么样.
 */

#include "led_gpio.h"
#include "stm32f4xx_hal.h"

/* ============== platform 层 (GPIO 封装函数) ==============
 *
 * pin 编码 / 解码宏跟 ch01 一字不变, port 表也是同一份.
 *   PIN_NUM('A', 13) = 0x0D, PIN_NUM('D', 12) = 0x3C, ...
 */
#define PIN_PORT_IDX(pin)     (((pin) >> 4) & 0x0F)
#define PIN_NO(pin)           ((pin) & 0x0F)
#define PIN_MASK(pin)         (1U << PIN_NO(pin))

static GPIO_TypeDef * const _gpio_table[] = {
	GPIOA, GPIOB, GPIOC, GPIOD, GPIOE,
#if defined(GPIOF)
	GPIOF,
#else
	NULL,
#endif
#if defined(GPIOG)
	GPIOG,
#else
	NULL,
#endif
#if defined(GPIOH)
	GPIOH,
#else
	NULL,
#endif
#if defined(GPIOI)
	GPIOI,
#else
	NULL,
#endif
};

#define PIN_PORT(pin)    (_gpio_table[PIN_PORT_IDX(pin)])

static void _enable_port_clock(uint8_t pin)
{
	switch (PIN_PORT_IDX(pin)) {
	case 0: __HAL_RCC_GPIOA_CLK_ENABLE(); break;
	case 1: __HAL_RCC_GPIOB_CLK_ENABLE(); break;
	case 2: __HAL_RCC_GPIOC_CLK_ENABLE(); break;
	case 3: __HAL_RCC_GPIOD_CLK_ENABLE(); break;
	case 4: __HAL_RCC_GPIOE_CLK_ENABLE(); break;
#if defined(__HAL_RCC_GPIOF_CLK_ENABLE)
	case 5: __HAL_RCC_GPIOF_CLK_ENABLE(); break;
#endif
#if defined(__HAL_RCC_GPIOG_CLK_ENABLE)
	case 6: __HAL_RCC_GPIOG_CLK_ENABLE(); break;
#endif
#if defined(__HAL_RCC_GPIOH_CLK_ENABLE)
	case 7: __HAL_RCC_GPIOH_CLK_ENABLE(); break;
#endif
#if defined(__HAL_RCC_GPIOI_CLK_ENABLE)
	case 8: __HAL_RCC_GPIOI_CLK_ENABLE(); break;
#endif
	default: break;
	}
}

void platform_gpio_init(uint8_t pin, uint8_t mode)
{
	GPIO_InitTypeDef cfg = {0};

	_enable_port_clock(pin);

	cfg.Pin   = PIN_MASK(pin);
	cfg.Mode  = (mode == GPIO_MODE_OUTPUT) ?
	            GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
	cfg.Pull  = GPIO_NOPULL;
	cfg.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(PIN_PORT(pin), &cfg);
}

void platform_gpio_deinit(uint8_t pin)
{
	HAL_GPIO_DeInit(PIN_PORT(pin), PIN_MASK(pin));
}

void platform_gpio_write(uint8_t pin, bool value)
{
	HAL_GPIO_WritePin(PIN_PORT(pin), PIN_MASK(pin),
	                  value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool platform_gpio_read(uint8_t pin)
{
	return HAL_GPIO_ReadPin(PIN_PORT(pin), PIN_MASK(pin)) == GPIO_PIN_SET;
}

/* ============== GPIO 子类: 拉 GPIO 电平 ==============
 *
 * pc/ 模拟版用 printf, STM32 版换成真实 platform_gpio_write. 函数名 /
 * 签名 / static 修饰都跟 pc/led_gpio.c 一字不变. ops 表通过 designated
 * initializer 把这些静态函数挂到 .on / .off / .toggle 字段, 落 .rodata
 * 段全程序唯一一份.
 */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	me->is_on = true;
	platform_gpio_write(self->pin, true);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	me->is_on = false;
	platform_gpio_write(self->pin, false);
	return 0;
}

static int gpio_toggle(struct led_base *me)
{
	if (me->is_on)
		return gpio_off(me);
	return gpio_on(me);
}

const struct led_ops led_ops_gpio = {
	.on     = gpio_on,
	.off    = gpio_off,
	.toggle = gpio_toggle,
};
