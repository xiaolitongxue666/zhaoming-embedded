/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.c - LED PWM 子类实现 (基于 Linux sysfs PWM 直接调用).
 *
 * sysfs PWM 路径布局 (/sys/class/pwm/pwmchipN/):
 *   export      写入 pwm_num 表示导出通道 (创建 pwm<pwm_num>/ 子目录)
 *   unexport    反过来释放
 *   pwm<N>/period       周期 (纳秒)
 *   pwm<N>/duty_cycle   占空时间 (纳秒)
 *   pwm<N>/enable       0/1 关开
 *
 * 上电默认 brightness = 255 (全亮). 后续 led_set_brightness 改 brightness,
 * 下次 led_on 用新值. led_off 把 enable 写 0.
 *
 * 注意: chip_num / pwm_num 哪个能用要看具体 SBC 的 device tree. 树莓派 4B
 * 默认 pwmchip0 + 通道 0/1. 出厂 device tree 不一定开启 PWM, 要在 config.txt
 * 加 dtoverlay=pwm-2chan 之类.
 */

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "drivers/led/led_pwm.h"

#define LED_PWM_DEFAULT_BRIGHTNESS    255
#define LED_PWM_PATH_MAX              128

static platform_err_t _led_pwm_on(struct led_base *me);
static platform_err_t _led_pwm_off(struct led_base *me);
static platform_err_t _led_pwm_set_brightness(
	struct led_base *me, uint8_t level);

static const struct led_ops led_pwm_ops = {
	.on             = _led_pwm_on,
	.off            = _led_pwm_off,
	.set_brightness = _led_pwm_set_brightness,
};

/* 把短字符串写到 sysfs 文件. 失败 -1. */
static int _write_str(const char *path, const char *val)
{
	int    fd;
	ssize_t n;

	fd = open(path, O_WRONLY);
	if (fd < 0) {
		return -1;
	}
	n = write(fd, val, strlen(val));
	(void)close(fd);
	if (n < 0) {
		return -1;
	}
	return 0;
}

/* 写整数 (十进制) 到 sysfs 文件. period_ns / duty_ns 都是 uint32_t. */
static int _write_uint(const char *path, uint32_t val)
{
	char buf[32];
	int  n;

	n = snprintf(buf, sizeof(buf), "%u", val);
	if ((n <= 0) || (n >= (int)sizeof(buf))) {
		return -1;
	}
	return _write_str(path, buf);
}

platform_err_t led_pwm_init(struct led_pwm *me, const char *name,
                            int chip_num, int pwm_num, uint32_t period_ns)
{
	char           export_path[LED_PWM_PATH_MAX];
	char           period_path[LED_PWM_PATH_MAX];
	char           duty_path[LED_PWM_PATH_MAX];
	char           enable_path[LED_PWM_PATH_MAX];
	char           num_buf[16];
	platform_err_t ret;

	if ((NULL == me) || (NULL == name) || (period_ns == 0)) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	(void)snprintf(export_path, sizeof(export_path),
	               "/sys/class/pwm/pwmchip%d/export", chip_num);
	(void)snprintf(period_path, sizeof(period_path),
	               "/sys/class/pwm/pwmchip%d/pwm%d/period",
	               chip_num, pwm_num);
	(void)snprintf(duty_path, sizeof(duty_path),
	               "/sys/class/pwm/pwmchip%d/pwm%d/duty_cycle",
	               chip_num, pwm_num);
	(void)snprintf(enable_path, sizeof(enable_path),
	               "/sys/class/pwm/pwmchip%d/pwm%d/enable",
	               chip_num, pwm_num);

	/* 1. export 通道 (已经 export 过会返回 EBUSY, 忽略) */
	(void)snprintf(num_buf, sizeof(num_buf), "%d", pwm_num);
	(void)_write_str(export_path, num_buf);

	/* 2. 设置 period (写 0 给 duty_cycle 防止 period < duty 报错) */
	(void)_write_uint(duty_path, 0U);
	if (_write_uint(period_path, period_ns) < 0) {
		fprintf(stderr, "[led_pwm:%s] set period failed (%s)\n",
		        name, period_path);
		ret = PLATFORM_EIO;
		goto exit;
	}

	/* 3. 打开 duty / enable fd, 后续 on/off/set_brightness 高频写 */
	me->duty_fd = open(duty_path, O_WRONLY);
	if (me->duty_fd < 0) {
		fprintf(stderr, "[led_pwm:%s] open duty failed: %s\n",
		        name, strerror(errno));
		ret = PLATFORM_EIO;
		goto exit;
	}

	me->enable_fd = open(enable_path, O_WRONLY);
	if (me->enable_fd < 0) {
		fprintf(stderr, "[led_pwm:%s] open enable failed: %s\n",
		        name, strerror(errno));
		(void)close(me->duty_fd);
		me->duty_fd = -1;
		ret = PLATFORM_EIO;
		goto exit;
	}

	me->chip_num   = chip_num;
	me->pwm_num    = pwm_num;
	me->period_ns  = period_ns;
	me->brightness = LED_PWM_DEFAULT_BRIGHTNESS;

	/* 4. 默认关闭 + duty=0 */
	(void)write(me->enable_fd, "0", 1);

	ret = led_base_init(&me->base, name, &led_pwm_ops);

exit:
	return ret;
}

void led_pwm_deinit(struct led_pwm *me)
{
	if (NULL == me) {
		return;
	}
	if (me->enable_fd >= 0) {
		(void)write(me->enable_fd, "0", 1);
		(void)close(me->enable_fd);
		me->enable_fd = -1;
	}
	if (me->duty_fd >= 0) {
		(void)close(me->duty_fd);
		me->duty_fd = -1;
	}
}

/* level (0-255) -> duty_ns (0-period_ns) */
static uint32_t _level_to_duty_ns(uint8_t level, uint32_t period_ns)
{
	return (uint32_t)((((uint64_t)level) * period_ns) / 255U);
}

static int _write_duty(struct led_pwm *pwm, uint8_t level)
{
	char    buf[32];
	int     n;
	ssize_t w;

	n = snprintf(buf, sizeof(buf), "%u",
	             _level_to_duty_ns(level, pwm->period_ns));
	if ((n <= 0) || (n >= (int)sizeof(buf))) {
		return -1;
	}
	if (lseek(pwm->duty_fd, 0, SEEK_SET) < 0) {
		return -1;
	}
	w = write(pwm->duty_fd, buf, (size_t)n);
	if (w < 0) {
		return -1;
	}
	return 0;
}

static platform_err_t _led_pwm_on(struct led_base *me)
{
	struct led_pwm *pwm = (struct led_pwm *)me;

	if (_write_duty(pwm, pwm->brightness) < 0) {
		return PLATFORM_EIO;
	}
	if (lseek(pwm->enable_fd, 0, SEEK_SET) < 0) {
		return PLATFORM_EIO;
	}
	if (write(pwm->enable_fd, "1", 1) < 0) {
		return PLATFORM_EIO;
	}
	return PLATFORM_EOK;
}

static platform_err_t _led_pwm_off(struct led_base *me)
{
	struct led_pwm *pwm = (struct led_pwm *)me;

	if (lseek(pwm->enable_fd, 0, SEEK_SET) < 0) {
		return PLATFORM_EIO;
	}
	if (write(pwm->enable_fd, "0", 1) < 0) {
		return PLATFORM_EIO;
	}
	return PLATFORM_EOK;
}

static platform_err_t _led_pwm_set_brightness(
	struct led_base *me, uint8_t level)
{
	struct led_pwm *pwm = (struct led_pwm *)me;

	pwm->brightness = level;

	/* 如果当前是亮的, 立刻把新亮度推下去, 否则只更新缓存值 */
	if (me->is_on) {
		if (_write_duty(pwm, level) < 0) {
			return PLATFORM_EIO;
		}
	}
	return PLATFORM_EOK;
}
