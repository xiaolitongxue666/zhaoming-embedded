/* SPDX-License-Identifier: MIT */
/*
 * led_stm32.c - 同一份 led_on / led_off 在 STM32 上长什么样
 *
 * 这是片段, 不是完整工程. 完整 STM32 工程见附录 B.
 * 用 STM32CubeMX 把对应的 GPIO 配成输出后, 下面的实现就能跑.
 *
 * 关键观察: led.h / led.c / main.c 一字不改, 上层代码 100% 复用.
 * 变化的只是这个 platform_*.c 文件. 这是平台抽象层最直接的威力.
 */

#include "led.h"
#include "stm32f4xx_hal.h"

/*
 * pin 编码: 一个 uint8_t 同时表示 port 和 pin 号.
 *   高 4 位 = port 索引 (A=0, B=1, C=2, ..., I=8)
 *   低 4 位 = pin 号 (0-15)
 *
 * 例:
 *   PA.13 = 0x0D    (port=0=A, num=13=0xD)
 *   PD.12 = 0x3C    (port=3=D, num=12=0xC)
 *   PI.14 = 0x8E    (port=8=I, num=14=0xE)
 *
 * 这套编码和 industrial/stm32_full/app/platform/arch/board/pin_board.c
 * 字节级一致. ch01 这里先用最少的代码把"换 port + 换 pin"跑通,
 * 工业版多一层"PA.13 字符串 -> 0x0D 编码"的解析, 核心编码不变.
 */
#define PIN_NUM(port, num)    ((((port) - 'A') << 4) | ((num) & 0x0F))
#define PIN_PORT_IDX(pin)     (((pin) >> 4) & 0x0F)
#define PIN_NO(pin)           ((pin) & 0x0F)
#define PIN_MASK(pin)         (1U << PIN_NO(pin))

/*
 * port 索引 -> GPIO_TypeDef *.
 * 用 BASE + 0x400 偏移也行 (industrial 版本就是), 这里用查表更直观.
 * 板子没用到的 port 在表里占一格 NULL, 永远查不到, 占用极小.
 */
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

/*
 * 按需开启对应 port 的时钟. STM32 上 GPIO port 时钟默认是关的, 不开
 * 时钟直接读写寄存器是没反应的. 这里只对实际用到的 port 开一次.
 */
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

/*
 * 应用层调用方式 (在 main.c 里):
 *
 *   struct led red_led;
 *   led_init(&red_led, 0x0D);   <-- PA.13
 *   led_on(&red_led);           <-- 和 PC 版完全一样
 *
 * pin = 0x0D / 0x0E / 0x0F 分别对应 PA.13 / PA.14 / PA.15.
 * 板子上 LED 接到别的 port 就传不同编码 (例 PD.12 = 0x3C),
 * led.c / main.c 一行不动.
 */

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

/*
 * 编译命令 (CubeIDE 或命令行 arm-none-eabi-gcc):
 *
 *   arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -DSTM32F407xx \
 *       -I... -o firmware.elf \
 *       main.c led.c led_stm32.c stm32f4xx_hal_*.c
 *
 * 烧录后真实 LED 会按 main.c 描述的顺序点亮.
 */
