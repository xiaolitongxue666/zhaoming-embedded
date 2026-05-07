/* SPDX-License-Identifier: MIT */
/*
 * platform_linux.c - ch14 Linux 用户态等效片段（函数式 platform · sysfs）
 *
 * 本文件是 Linux 用户态真实实现片段。
 * 需要内核启用 sysfs gpio / pwm + i2c-dev 模块。
 * 用法: 把 gpio / pwm / i2c 节点 export 出来后, 用户态进程读写文件 /
 * ioctl 操作硬件。
 *
 * 替换 ch14 pc/ 里 PC 模拟版的 4 个 platform 封装函数实现。led.c / main.c
 * / container_of.h 一字不动。
 *
 * 必填 / 选填 / 全必填三种 ops 表策略和 platform 在哪个平台无关。assert
 * 在 Linux 用户态调试构建里照样 abort，glibc 把诊断信息打到 stderr。
 *
 * 本章主线是 led_ops 这一层（子类层）的三种策略, platform 只是稳定背景,
 * 所以这里直接 4 个函数把 sysfs 包一层. PWM 子类 (channel + duty) 在
 * Linux 上走 sysfs pwmchipN/pwmM, 三件套 ops (pwm_on / pwm_off /
 * pwm_set_brightness) 都给出 Linux 等效实现, 演示"全填"策略落到真硬件
 * 上的样子.
 */

#include "led.h"
#include "platform.h"
#include "container_of.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ============== platform_gpio_* (sysfs gpio 实现) ============== */

void platform_gpio_init(uint8_t pin, uint8_t mode)
{
	char path[64];
	int fd;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd >= 0) {
		dprintf(fd, "%u", (unsigned)pin);
		close(fd);
	}
	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%u/direction", (unsigned)pin);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		const char *dir = (mode == GPIO_MODE_OUTPUT) ? "out" : "in";
		write(fd, dir, strlen(dir));
		close(fd);
	}
}

void platform_gpio_deinit(uint8_t pin)
{
	int fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd >= 0) {
		dprintf(fd, "%u", (unsigned)pin);
		close(fd);
	}
}

void platform_gpio_write(uint8_t pin, bool value)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%u/value", (unsigned)pin);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, value ? "1" : "0", 1);
		close(fd);
	}
}

bool platform_gpio_read(uint8_t pin)
{
	char path[64], buf[2] = {0};
	int fd;

	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%u/value", (unsigned)pin);
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return false;
	read(fd, buf, 1);
	close(fd);
	return buf[0] == '1';
}

/* ============== PWM 子类 ops 在 Linux 上的实现 (三件套全填) ==========
 *
 * sysfs pwmchip0/pwmN 节点 (CONFIG_PWM_SYSFS=y) 三个文件: period /
 * duty_cycle / enable. period 先于 duty_cycle 写, duty_cycle <= period.
 * enable 在 period / duty_cycle 都设好之后再写, 否则 -EINVAL.
 *
 * 三件套 (on / off / set_brightness) 在 PWM 子类全部填实, 演示 ch14
 * "全必填"策略 (在 led_ops 上是"on/off 必填 + set_brightness 选填",
 * PWM 选择全填以兑现调光能力).
 */

static int pwm_apply_duty(uint8_t channel, uint8_t duty)
{
	char path[64];
	int fd;
	unsigned long period_ns = 1000000UL;                  /* 1 kHz */
	unsigned long duty_ns   = period_ns * duty / 100UL;   /* 0..100 % */

	snprintf(path, sizeof(path),
		 "/sys/class/pwm/pwmchip0/pwm%u/period", (unsigned)channel);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;
	dprintf(fd, "%lu", period_ns);
	close(fd);

	snprintf(path, sizeof(path),
		 "/sys/class/pwm/pwmchip0/pwm%u/duty_cycle", (unsigned)channel);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;
	dprintf(fd, "%lu", duty_ns);
	close(fd);
	return 0;
}

static int pwm_apply_enable(uint8_t channel, bool on)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
		 "/sys/class/pwm/pwmchip0/pwm%u/enable", (unsigned)channel);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;
	write(fd, on ? "1" : "0", 1);
	close(fd);
	return 0;
}

int pwm_on_linux(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	pwm_apply_duty(self->channel, self->duty);
	pwm_apply_enable(self->channel, true);
	me->is_on = true;
	return 0;
}

int pwm_off_linux(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	pwm_apply_enable(self->channel, false);
	me->is_on = false;
	return 0;
}

int pwm_set_brightness_linux(struct led_base *me, uint8_t brightness)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	if (brightness > 100)
		brightness = 100;
	self->duty = brightness;
	pwm_apply_duty(self->channel, brightness);
	me->is_on = (brightness > 0);
	return 0;
}
