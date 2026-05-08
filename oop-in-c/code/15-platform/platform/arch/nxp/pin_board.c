/* SPDX-License-Identifier: MIT */
/*
 * platform/arch/nxp/pin_board.c - NXP i.MX RT1170 端 PIN 后端实现.
 *
 * 和 platform/arch/stm32/pin_board.c 同款角色: "认识 NXP GPIO 寄存器"的
 * 唯一一份文件. 上层 drivers/led/led_gpio 调 platform_pin_xxx ops 表层
 * 接口下来, 落到这里之后才碰 GPIO_Type * 加 GPIO_PinWrite.
 *
 * 跨 MCU (STM32 -> NXP) 的"换 6 份之一": 本文 + 同目录 pwm_board.c +
 * i2c_board.c 三份是这家 MCU 的 platform 实现层. 上层 drivers/led 下所有
 * 子类 + platform 接口层 .h/.c 字节级不动. 这是 ch15 § 15.11.5 "STM32 vs NXP"
 * 的代码兑现点.
 *
 * 唯一不同: 厂家 SDK 类型 / API 名字
 *   GPIO_TypeDef *           ->  GPIO_Type *           (NXP MCUXpresso SDK)
 *   HAL_GPIO_WritePin        ->  GPIO_PinWrite
 *   GPIO_InitTypeDef         ->  gpio_pin_config_t
 *   __HAL_RCC_GPIOx_CLK_ENABLE -> CLOCK_EnableClock(kCLOCK_Gpio1) 风格
 *
 * 工程假设: MCUXpresso IDE 已经做完 pin mux + clock 配置, fsl_gpio 头
 * 是 SDK builtin.
 */

#include "platform/platform_pin.h"
#include "fsl_gpio.h"

#include <string.h>

/* PIN 编码: 高 4 位 port, 低 4 位 num. NXP 的 GPIO1..GPIO9 对应 port 0..8. */
#define PIN_PORT(pin)        ((uint8_t)(((pin) >> 4) & 0x0F))
#define PIN_OFFSET(pin)      ((uint8_t)((pin) & 0x0F))

/* port 解码表: 0..4 -> GPIO1..GPIO5. */
static GPIO_Type *const _gpio_table[] = {
	GPIO1, GPIO2, GPIO3, GPIO4, GPIO5,
};

#define _GPIO_TABLE_SIZE  (sizeof(_gpio_table) / sizeof(_gpio_table[0]))

static int32_t _nxp_pin_get(const char *name)
{
	int port_num;
	int pin_num = 0;
	int len;
	int i;

	if (!name)
		return -1;
	len = (int)strlen(name);
	if (len < 4 || len >= 6)
		return -1;
	if (name[0] != 'P' || name[2] != '.')
		return -1;
	if (name[1] < 'A' || name[1] > 'Z')
		return -1;

	port_num = name[1] - 'A';
	for (i = 3; i < len; i++)
		pin_num = pin_num * 10 + (name[i] - '0');

	return ((port_num & 0x0F) << 4) | (pin_num & 0x0F);
}

static void _nxp_pin_mode(int32_t pin, int32_t mode)
{
	GPIO_Type        *port = _gpio_table[PIN_PORT(pin)];
	gpio_pin_config_t cfg  = {0};

	if (PIN_PORT(pin) >= _GPIO_TABLE_SIZE)
		return;
	cfg.outputLogic = 0U;
	switch (mode) {
	case PIN_MODE_OUTPUT: cfg.direction = kGPIO_DigitalOutput; break;
	case PIN_MODE_INPUT:  cfg.direction = kGPIO_DigitalInput;  break;
	default: return;
	}
	GPIO_PinInit(port, PIN_OFFSET(pin), &cfg);
}

static void _nxp_pin_write(int32_t pin, int32_t value)
{
	GPIO_Type *port = _gpio_table[PIN_PORT(pin)];

	if (PIN_PORT(pin) >= _GPIO_TABLE_SIZE)
		return;
	GPIO_PinWrite(port, PIN_OFFSET(pin), value ? 1U : 0U);
}

static int32_t _nxp_pin_read(int32_t pin)
{
	GPIO_Type *port = _gpio_table[PIN_PORT(pin)];

	if (PIN_PORT(pin) >= _GPIO_TABLE_SIZE)
		return PIN_LOW;
	return GPIO_PinRead(port, PIN_OFFSET(pin)) ? PIN_HIGH : PIN_LOW;
}

static const struct platform_pin_ops _nxp_pin_ops = {
	.mode  = _nxp_pin_mode,
	.write = _nxp_pin_write,
	.read  = _nxp_pin_read,
	.get   = _nxp_pin_get,
};

/* 启动期由 platform_init 调一次. MCUXpresso BOARD_InitPins / BOARD_BootClockRUN
 * 已经做完 mux + clock, 这里只把 ops 注册到 dispatcher. */
void platform_hw_pin_init(void)
{
	(void)platform_pin_register(&_nxp_pin_ops);
}
