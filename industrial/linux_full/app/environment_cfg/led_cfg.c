/* SPDX-License-Identifier: MIT */
/*
 * led_cfg.c - LED 实例装配 (Linux 用户态版本).
 *
 * 配置层负责装配 LED 实例 (静态分配 + 调子类 init), 导出 struct led_base *
 * 句柄给应用层. 应用层调用形态:
 *
 *   if (environment_init() != 0) { ...exit... }
 *   ... led_on(led_status) / led_set_brightness(led_dimmer, 128) ...
 *   environment_exit();
 *
 * Linux 用户态没有 Linux 内核 initcall, 不要套自己的 initcall: main 显式调
 * 一次, 控制流清晰. 装配失败时上层根据 environment_init() 的返回值决定是
 * 否中止运行.
 *
 * 这一份是 3 种子类混搭的演示, 应用层只见 struct led_base *, 看不到子类
 * 完整类型. 板子换了 / 硬件换了 / 控制方式从 GPIO 换 I2C, 改这一份的实例
 * 装配, 上层 driver / 应用层一字不动.
 */

#include <gpiod.h>
#include <stddef.h>
#include <stdio.h>

#include "drivers/led/led_gpio.h"
#include "drivers/led/led_i2c.h"
#include "drivers/led/led_pwm.h"
#include "environment_cfg/environment_export.h"

/* 板上资源参数. 不同 SBC 改这一组常量即可. */
#define LED_GPIO_CHIP_NAME        "gpiochip0"
#define LED_STATUS_LINE           17        /* BCM 17 */
#define LED_ALARM_LINE            22        /* BCM 22 */
#define LED_PWM_CHIP              0          /* /sys/class/pwm/pwmchip0 */
#define LED_PWM_NUM               0          /* pwm0 */
#define LED_PWM_PERIOD_NS         1000000U   /* 1 kHz */
#define LED_I2C_BUS               1          /* /dev/i2c-1 */
#define LED_I2C_ADDR              0x3C
#define LED_I2C_REG               0x00

/* 应用层句柄: 4 颗 LED, 全部 struct led_base * */
struct led_base *led_status = NULL;
struct led_base *led_dimmer = NULL;
struct led_base *led_panel  = NULL;
struct led_base *led_alarm  = NULL;

/* 静态实例池: 子类完整类型, static 让应用层看不到 */
static struct led_gpio    _led_status_inst;
static struct led_pwm     _led_dimmer_inst;
static struct led_i2c     _led_panel_inst;
static struct led_gpio    _led_alarm_inst;

/* libgpiod chip 句柄, 两颗 GPIO LED 共用. */
static struct gpiod_chip *_g_chip;

int environment_init(void)
{
	platform_err_t ret;

	/* fd 默认 -1, deinit 不会 close 错误 fd. */
	_led_dimmer_inst.duty_fd   = -1;
	_led_dimmer_inst.enable_fd = -1;
	_led_panel_inst.fd         = -1;

	_g_chip = gpiod_chip_open_by_name(LED_GPIO_CHIP_NAME);
	if (NULL == _g_chip) {
		fprintf(stderr,
		        "[env] gpiod_chip_open_by_name(%s) failed\n",
		        LED_GPIO_CHIP_NAME);
		goto fail;
	}

	ret = led_gpio_init(&_led_status_inst, "status",
	                    _g_chip, LED_STATUS_LINE, true);
	if (PLATFORM_EOK != ret) {
		fprintf(stderr, "[env] led_status init failed: %d\n", (int)ret);
		goto fail;
	}
	led_status = &_led_status_inst.base;

	ret = led_pwm_init(&_led_dimmer_inst, "dimmer",
	                   LED_PWM_CHIP, LED_PWM_NUM, LED_PWM_PERIOD_NS);
	if (PLATFORM_EOK != ret) {
		fprintf(stderr, "[env] led_dimmer init failed: %d\n", (int)ret);
		goto fail;
	}
	led_dimmer = &_led_dimmer_inst.base;

	ret = led_i2c_init(&_led_panel_inst, "panel",
	                   LED_I2C_BUS, LED_I2C_ADDR, LED_I2C_REG);
	if (PLATFORM_EOK != ret) {
		fprintf(stderr, "[env] led_panel init failed: %d\n", (int)ret);
		goto fail;
	}
	led_panel = &_led_panel_inst.base;

	ret = led_gpio_init(&_led_alarm_inst, "alarm",
	                    _g_chip, LED_ALARM_LINE, true);
	if (PLATFORM_EOK != ret) {
		fprintf(stderr, "[env] led_alarm init failed: %d\n", (int)ret);
		goto fail;
	}
	led_alarm = &_led_alarm_inst.base;

	return 0;

fail:
	environment_exit();
	return -1;
}

void environment_exit(void)
{
	led_gpio_deinit(&_led_status_inst);
	led_gpio_deinit(&_led_alarm_inst);
	led_pwm_deinit(&_led_dimmer_inst);
	led_i2c_deinit(&_led_panel_inst);

	if (NULL != _g_chip) {
		gpiod_chip_close(_g_chip);
		_g_chip = NULL;
	}

	led_status = NULL;
	led_dimmer = NULL;
	led_panel  = NULL;
	led_alarm  = NULL;
}
