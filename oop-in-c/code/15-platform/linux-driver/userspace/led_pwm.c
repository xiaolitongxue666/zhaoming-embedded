/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.c - 子类二: PWM LED 实现 (ch15 linux-driver/userspace 版).
 *
 * sysfs PWM 路径布局 (/sys/class/pwm/pwmchipN/):
 *   export        写 pwm_num 表示导出通道, 创建 pwm<pwm_num>/ 子目录
 *   pwm<N>/period       周期 (ns)
 *   pwm<N>/duty_cycle   占空时间 (ns)
 *   pwm<N>/enable       0/1 关 / 开
 *
 * 注: chip_num / pwm_num 哪个能用要看具体 SBC 的 device tree. 树莓派 4B
 * 默认 pwmchip0 + 通道 0/1. 出厂 device tree 不一定开 PWM, 要在 config.txt
 * 加 dtoverlay=pwm-2chan 之类.
 */

#include "led_pwm.h"
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define LED_PWM_DEFAULT_BRIGHTNESS  255
#define LED_PWM_PATH_MAX            128

static int write_str(const char *path, const char *val)
{
	int     fd;
	ssize_t n;

	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;
	n = write(fd, val, strlen(val));
	close(fd);
	return (n < 0) ? -1 : 0;
}

static int write_uint(const char *path, uint32_t val)
{
	char buf[32];
	int  n;

	n = snprintf(buf, sizeof(buf), "%u", val);
	if (n <= 0 || n >= (int)sizeof(buf))
		return -1;
	return write_str(path, buf);
}

static uint32_t level_to_duty_ns(uint8_t level, uint32_t period_ns)
{
	return (uint32_t)((((uint64_t)level) * period_ns) / 255U);
}

static int write_duty(struct led_pwm *self, uint8_t level)
{
	char    buf[32];
	int     n;
	ssize_t w;

	n = snprintf(buf, sizeof(buf), "%u",
	             level_to_duty_ns(level, self->period_ns));
	if (n <= 0 || n >= (int)sizeof(buf))
		return -1;
	if (lseek(self->duty_fd, 0, SEEK_SET) < 0)
		return -1;
	w = write(self->duty_fd, buf, (size_t)n);
	return (w < 0) ? -1 : 0;
}

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;

	if (write_duty(self, self->brightness) < 0)
		return -1;
	if (lseek(self->enable_fd, 0, SEEK_SET) < 0)
		return -1;
	if (write(self->enable_fd, "1", 1) < 0)
		return -1;
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;

	if (lseek(self->enable_fd, 0, SEEK_SET) < 0)
		return -1;
	if (write(self->enable_fd, "0", 1) < 0)
		return -1;
	me->is_on = false;
	return 0;
}

static int pwm_set_brightness(struct led_base *me, uint8_t level)
{
	struct led_pwm *self = (struct led_pwm *)me;

	self->brightness = level;
	if (me->is_on) {
		if (write_duty(self, level) < 0)
			return -1;
	}
	return 0;
}

static const struct led_ops pwm_ops = {
	.on             = pwm_on,
	.off            = pwm_off,
	.set_brightness = pwm_set_brightness,
};

int led_pwm_init(struct led_pwm *me, const char *name,
                 int chip_num, int pwm_num, uint32_t period_ns)
{
	char export_path[LED_PWM_PATH_MAX];
	char period_path[LED_PWM_PATH_MAX];
	char duty_path[LED_PWM_PATH_MAX];
	char enable_path[LED_PWM_PATH_MAX];
	char num_buf[16];

	if (!me || !name || period_ns == 0U)
		return -1;

	snprintf(export_path, sizeof(export_path),
	         "/sys/class/pwm/pwmchip%d/export", chip_num);
	snprintf(period_path, sizeof(period_path),
	         "/sys/class/pwm/pwmchip%d/pwm%d/period", chip_num, pwm_num);
	snprintf(duty_path, sizeof(duty_path),
	         "/sys/class/pwm/pwmchip%d/pwm%d/duty_cycle", chip_num, pwm_num);
	snprintf(enable_path, sizeof(enable_path),
	         "/sys/class/pwm/pwmchip%d/pwm%d/enable", chip_num, pwm_num);

	/* 1. export 通道 (已 export 会 EBUSY, 忽略) */
	snprintf(num_buf, sizeof(num_buf), "%d", pwm_num);
	(void)write_str(export_path, num_buf);

	/* 2. 设 period (写 0 给 duty_cycle 防 period < duty 报错) */
	(void)write_uint(duty_path, 0U);
	if (write_uint(period_path, period_ns) < 0) {
		fprintf(stderr, "[led_pwm:%s] set period failed (%s)\n",
		        name, period_path);
		return -1;
	}

	/* 3. 打开 duty / enable fd, 后续高频写 */
	me->duty_fd = open(duty_path, O_WRONLY);
	if (me->duty_fd < 0) {
		fprintf(stderr, "[led_pwm:%s] open duty failed: %s\n",
		        name, strerror(errno));
		return -1;
	}
	me->enable_fd = open(enable_path, O_WRONLY);
	if (me->enable_fd < 0) {
		fprintf(stderr, "[led_pwm:%s] open enable failed: %s\n",
		        name, strerror(errno));
		close(me->duty_fd);
		me->duty_fd = -1;
		return -1;
	}

	me->chip_num   = chip_num;
	me->pwm_num    = pwm_num;
	me->period_ns  = period_ns;
	me->brightness = LED_PWM_DEFAULT_BRIGHTNESS;

	/* 4. 默认关闭 + duty=0 */
	(void)write(me->enable_fd, "0", 1);

	return led_base_init(&me->base, name, &pwm_ops);
}

void led_pwm_deinit(struct led_pwm *me)
{
	if (!me)
		return;
	if (me->enable_fd >= 0) {
		(void)write(me->enable_fd, "0", 1);
		close(me->enable_fd);
		me->enable_fd = -1;
	}
	if (me->duty_fd >= 0) {
		close(me->duty_fd);
		me->duty_fd = -1;
	}
}
