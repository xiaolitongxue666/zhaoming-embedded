/* SPDX-License-Identifier: MIT */
/*
 * led_cfg.c - LED 实例配置 + initcall 注册.
 *
 * 配置层负责装配 LED 实例 (静态分配 + 调子类 init), 导出 struct led_base *
 * 句柄给应用层. 启动期通过 INIT_ENV_EXPORT 自动调用 (5 级 ENV, 排在
 * BOARD/PREV/DEVICE/COMPONENT 之后, APP 之前).
 *
 * 这一份是 3 种子类混搭的演示, 应用层只见 struct led_base *, 看不到子类
 * 完整类型. 板子换了 / 硬件换了 / 控制方式从 GPIO 换 I2C, 改这一份的实例
 * 装配, 上层 driver / 应用层一字不动.
 */

#include <stddef.h>
#include <stdio.h>

#include "drivers/led/led_gpio.h"
#include "drivers/led/led_i2c.h"
#include "drivers/led/led_pwm.h"
#include "environment_cfg/environment_export.h"
#include "platform/platform_i2c.h"
#include "platform/platform_module_export.h"

/* 板级注册的 I2C bus 句柄. 双模 build 各自指向自己的实例:
 *   MOCK_PC          -> mock/i2c_board_pc.c   定义的 pc_i2c_bus0
 *   非 MOCK_PC (真机) -> arch/board/i2c_board.c 定义的 stm32_i2c1_bus
 * 应用层只看到 _led_panel_bus 一个名字, 换板换 MCU 不用动. */
#ifdef MOCK_PC
extern struct platform_i2c_bus_device pc_i2c_bus0;
#define _led_panel_bus    (&pc_i2c_bus0)
#else
extern struct platform_i2c_bus_device stm32_i2c1_bus;
#define _led_panel_bus    (&stm32_i2c1_bus)
#endif

/* 应用层句柄: 4 颗 LED, 全部 struct led_base * */
struct led_base *led_status = NULL;
struct led_base *led_dimmer = NULL;
struct led_base *led_panel  = NULL;
struct led_base *led_alarm  = NULL;

/* 静态实例池: 子类完整类型, static 让应用层看不到 */
static struct led_gpio _led_status_inst;
static struct led_pwm  _led_dimmer_inst;
static struct led_i2c  _led_panel_inst;
static struct led_gpio _led_alarm_inst;

static void _led_cfg(void)
{
	platform_err_t ret;

	ret = led_gpio_init(&_led_status_inst, "status", "PD.12", true);
	if (PLATFORM_EOK != ret) {
		printf("[led_cfg] status init failed: %d\n", (int)ret);
		goto exit;
	}
	led_status = &_led_status_inst.base;

	ret = led_pwm_init(&_led_dimmer_inst, "dimmer", 0);
	if (PLATFORM_EOK != ret) {
		printf("[led_cfg] dimmer init failed: %d\n", (int)ret);
		goto exit;
	}
	led_dimmer = &_led_dimmer_inst.base;

	ret = led_i2c_init(&_led_panel_inst, "panel", _led_panel_bus, 0x3C, 0x00);
	if (PLATFORM_EOK != ret) {
		printf("[led_cfg] panel init failed: %d\n", (int)ret);
		goto exit;
	}
	led_panel = &_led_panel_inst.base;

	ret = led_gpio_init(&_led_alarm_inst, "alarm", "PD.15", true);
	if (PLATFORM_EOK != ret) {
		printf("[led_cfg] alarm init failed: %d\n", (int)ret);
		goto exit;
	}
	led_alarm = &_led_alarm_inst.base;

exit:
	return;
}
INIT_ENV_EXPORT(_led_cfg);
