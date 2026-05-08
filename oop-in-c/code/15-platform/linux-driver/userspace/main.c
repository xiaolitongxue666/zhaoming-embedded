/* SPDX-License-Identifier: MIT */
/*
 * main.c - ch15 LED 框架在 Linux 用户态长什么样.
 *
 * 应用层只看到 led_on / led_off / led_set_brightness 这三个父类接口,
 * 拿到的是 struct led_base * 句柄. 板级层 (这里是 main 里面 init 几行)
 * 选 GPIO / PWM / I2C 三种子类混搭. 跟 PC 教学版 / MCU 真机版应用层
 * 写法字节级一致.
 *
 * 唯一差别: GPIO 子类构造接受 struct gpiod_chip *, PWM 子类构造接受
 * pwmchip / pwm_num, I2C 子类构造接受 bus / addr -- 这些是"哪条线"
 * 信息, 跟 led_board_init 写在哪份文件里无关.
 *
 * 真机跑 (树莓派 4B 默认 device tree, 默认 /dev/gpiochip0 是 BCM GPIO):
 *   sudo ./demo
 */

#include "led_base.h"
#include "led_gpio.h"
#include "led_pwm.h"
#include "led_i2c.h"
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	struct gpiod_chip *chip;
	struct led_gpio    s_led_err  = {0};
	struct led_pwm     s_led_stat = {0};
	struct led_i2c     s_led_net  = {0};
	struct led_base   *g_led_error;
	struct led_base   *g_led_status;
	struct led_base   *g_led_network;
	int rc;

	printf("=========================================\n");
	printf("  ch15 - Linux userspace LED framework\n");
	printf("=========================================\n");

	chip = gpiod_chip_open_by_name("gpiochip0");
	if (!chip) {
		fprintf(stderr, "[main] open gpiochip0 failed (need root?)\n");
		return 1;
	}

	rc = led_gpio_init(&s_led_err, "ERR", chip, 17, true);
	if (rc != 0)
		goto out_chip;

	rc = led_pwm_init(&s_led_stat, "STAT", 0, 0, 1000000U);   /* 1 kHz */
	if (rc != 0)
		goto out_gpio;

	rc = led_i2c_init(&s_led_net, "NET", 1, 0x20, 0x00);
	if (rc != 0)
		goto out_pwm;

	g_led_error   = &s_led_err.base;
	g_led_status  = &s_led_stat.base;
	g_led_network = &s_led_net.base;

	led_on(g_led_error);
	sleep(1);
	led_off(g_led_error);

	led_set_brightness(g_led_status, 128);
	led_on(g_led_status);
	sleep(1);
	led_off(g_led_status);

	led_on(g_led_network);
	sleep(1);
	led_off(g_led_network);

	led_i2c_deinit(&s_led_net);
out_pwm:
	led_pwm_deinit(&s_led_stat);
out_gpio:
	led_gpio_deinit(&s_led_err);
out_chip:
	gpiod_chip_close(chip);
	return rc ? 1 : 0;
}
