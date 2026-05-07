/* SPDX-License-Identifier: MIT */
/**
 * @file  led_linux.c
 * @brief ops 表在 Linux 用户态的平台胶水
 *
 * @details
 * 本文件是 Linux 用户态真实实现片段。
 * 需要内核启用 sysfs gpio / pwm + i2c-dev 模块。
 * 用法: 把 gpio / pwm / i2c 节点 export 出来后, 用户态进程读写文件 /
 * ioctl 操作硬件。
 *
 * 实现 platform.h 里的 platform_gpio_xxx 封装函数, 走 sysfs.
 * led_base.h / led.h / led.c / main.c 一字不改.
 *
 * gcc / clang 把 const struct led_ops 放进 .rodata, 进程加载时
 * 在虚拟内存里只读映射, 全进程共享 (单实例).
 *
 * 本文件还提供 PWM 子类的 on/off/toggle 在 Linux 用户态的真身: sysfs
 * pwmchipN/pwmM 节点写 period / duty_cycle / enable. 子类签名仍是
 * int xxx(struct led_base *me), 第一行 (struct led_pwm *)me 强转回
 * 子类拿到 channel / duty 字段. ops 表 led_ops_pwm 定义在 led.c, 这里
 * 给出 Linux 等效的实现函数, 让读者看到"换硬件不换接口"在 Linux 上
 * 长什么样.
 */

#include "led.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
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
	char path[64], buf[2] = { 0 };
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

/* ============== PWM 子类 ops 在 Linux 上的实现 ==============
 *
 * sysfs pwmchip0/pwmN 节点用法 (内核 CONFIG_PWM_SYSFS=y):
 *   echo N        > /sys/class/pwm/pwmchip0/export
 *   echo 1000000  > /sys/class/pwm/pwmchip0/pwmN/period       (1 kHz)
 *   echo 500000   > /sys/class/pwm/pwmchip0/pwmN/duty_cycle   (50%)
 *   echo 1        > /sys/class/pwm/pwmchip0/pwmN/enable
 *
 * 写入顺序: period 必须先于 duty_cycle, duty_cycle <= period.
 * enable 在 period / duty_cycle 都设好之后再写, 否则 -EINVAL.
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
	struct led_pwm *self = (struct led_pwm *)me;
	pwm_apply_duty(self->channel, self->duty);
	pwm_apply_enable(self->channel, true);
	me->is_on = true;
	return 0;
}

int pwm_off_linux(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	pwm_apply_enable(self->channel, false);
	me->is_on = false;
	return 0;
}

int pwm_toggle_linux(struct led_base *me)
{
	if (me->is_on)
		return pwm_off_linux(me);
	return pwm_on_linux(me);
}
