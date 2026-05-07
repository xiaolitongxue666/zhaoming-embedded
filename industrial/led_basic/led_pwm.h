/* SPDX-License-Identifier: MIT */
/*
 * led_pwm.h - LED PWM 子类 (走 platform_pwm 调亮度).
 *
 * 这一份演示"子类追加新能力": 在 on/off 之外补 set_brightness, 父类的
 * led_set_brightness dispatch 会调到这里. GPIO 子类不实现, 走父类默认 no-op,
 * 应用层一行代码切实例就拿到了对应能力.
 */

#ifndef __LED_PWM_H
#define __LED_PWM_H

#include <stdint.h>

#include "led_base.h"
#include "platform_def.h"

struct led_pwm {
	struct led_base base;
	int32_t         channel;     /* PWM 通道 */
	uint8_t         brightness;  /* 当前亮度 0-255, on 时写入 */
};

/* 构造函数.
 *   me        子类实例
 *   name      实例名
 *   channel   PWM 通道号 (平台层细节)
 */
platform_err_t led_pwm_init(struct led_pwm *me, const char *name,
                            int32_t channel);

#endif /* __LED_PWM_H */
