/* SPDX-License-Identifier: MIT */
/*
 * platform_pin.h - PIN 抽象层 (ops 表 + register 机制).
 *
 * 字符串名 -> pin 号的工业级 GPIO 抽象层. 上层 driver / 应用层调用
 * platform_pin_xxx 封装函数, 看不到底层 GPIO_TypeDef * 寄存器或 port 索引.
 * 跨 MCU 移植时只换 platform/arch/<mcu>/pin_board.c 这一份, 上层一字不动.
 *
 * 启动期由 platform/arch/<mcu>/pin_board.c 调 platform_pin_register
 * 把这家 MCU 的 ops 表填进 dispatcher.
 *
 * 见 ch15 § 15.11.5 "STM32 vs NXP 换 MCU 不改应用".
 */

#ifndef PLATFORM_PLATFORM_PIN_H
#define PLATFORM_PLATFORM_PIN_H

#include <stdint.h>

/* PIN 电平 */
#define PIN_LOW                0
#define PIN_HIGH               1

/* PIN 工作模式 */
#define PIN_MODE_OUTPUT        0
#define PIN_MODE_INPUT         1
#define PIN_MODE_INPUT_PULLUP  2

/* ops 表抽象 (子类填写) */
struct platform_pin_ops {
	void    (*mode)(int32_t pin, int32_t mode);
	void    (*write)(int32_t pin, int32_t value);
	int32_t (*read)(int32_t pin);
	int32_t (*get)(const char *name);   /* "PA.5" -> pin 号 */
};

/* 注册接口 (子类启动期用) */
int platform_pin_register(const struct platform_pin_ops *ops);

/* 公共 API (上层调) */
void    platform_pin_mode(int32_t pin, int32_t mode);
void    platform_pin_write(int32_t pin, int32_t value);
int32_t platform_pin_read(int32_t pin);

/* "PA.5" / "PD.12" -> pin 号 */
int32_t platform_pin_get(const char *name);

#endif /* PLATFORM_PLATFORM_PIN_H */
