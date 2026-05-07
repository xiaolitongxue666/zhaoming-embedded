/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.h - 子类二: PWM LED (ch15 linux-driver/userspace 版).
 *
 * 直接走 sysfs PWM 节点 (/sys/class/pwm/pwmchipN/pwmM/{period,duty_cycle,enable}).
 * 没有 platform_pwm_xxx 中间层 -- 内核 pwm subsystem 已经做完.
 *
 * 在 init 阶段把 duty_fd / enable_fd 打开缓存住, 后续 led_on / led_off /
 * set_brightness 高频写时不再 open / close, 一次系统调用直接 write.
 */

#ifndef LED_PWM_H
#define LED_PWM_H

#include "led_base.h"

struct led_pwm {
	struct led_base base;        /* 父类, 第 0 字段 */
	int             chip_num;    /* pwmchip 编号 */
	int             pwm_num;     /* 通道编号 */
	uint32_t        period_ns;   /* 周期 ns, 1e6 = 1 kHz */
	uint8_t         brightness;  /* 0-255 */
	int             duty_fd;
	int             enable_fd;
};

int  led_pwm_init(struct led_pwm *me, const char *name,
                  int chip_num, int pwm_num, uint32_t period_ns);
void led_pwm_deinit(struct led_pwm *me);

#endif /* LED_PWM_H */
