/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.h - LED PWM 子类 (基于 Linux sysfs PWM 实现).
 *
 * 直接走 /sys/class/pwm/pwmchipN/pwmM/ 接口, 不再经过任何 platform 抽象层.
 * sysfs PWM 是内核 PWM subsystem 暴露给用户态的标准接口, 用户态再套一层是
 * 过度封装.
 *
 * 这一份演示"子类追加新能力": 在 on/off 之外补 set_brightness, 父类的
 * led_set_brightness dispatch 会调到这里. GPIO 子类不实现, 走父类默认 no-op,
 * 应用层一行代码切实例就拿到了对应能力.
 */

#ifndef DRIVERS_LED_LED_PWM_H_
#define DRIVERS_LED_LED_PWM_H_

#include <stdint.h>

#include "drivers/led/led_base.h"
#include "led_errors.h"

struct led_pwm {
	struct led_base base;
	int             chip_num;       /* /sys/class/pwm/pwmchip<chip_num> */
	int             pwm_num;        /* pwm<pwm_num> */
	int             duty_fd;        /* open .../duty_cycle, O_WRONLY */
	int             enable_fd;      /* open .../enable,     O_WRONLY */
	uint32_t        period_ns;      /* PWM 周期 (纳秒) */
	uint8_t         brightness;     /* 当前亮度 0-255, on 时写入 */
};

/* 构造函数.
 *   me         子类实例
 *   name       实例名
 *   chip_num   sysfs 上 pwmchip 编号 (树莓派 4B 一般是 0)
 *   pwm_num    chip 下的 pwm 通道号 (一般 0 / 1)
 *   period_ns  PWM 周期 (纳秒). 1 kHz LED 调光: 1000000.
 *
 * 内部调用顺序:
 *   1. write /sys/class/pwm/pwmchipN/export = pwm_num   (导出通道)
 *   2. write .../pwmM/period = period_ns
 *   3. open .../pwmM/duty_cycle, .../pwmM/enable (保存 fd, 后续高频写)
 *   4. led_base_init 填 ops 表
 *
 * 失败 PLATFORM_EIO. /sys/class/pwm/ 不存在 (内核没编 PWM driver) 也是 EIO.
 */
platform_err_t led_pwm_init(struct led_pwm *me, const char *name,
                            int chip_num, int pwm_num, uint32_t period_ns);

/* 关掉 PWM, 关 fd. unexport 留给用户决定 (持有的 pwm 通道有时下次要复用). */
void led_pwm_deinit(struct led_pwm *me);

#endif /* DRIVERS_LED_LED_PWM_H_ */
