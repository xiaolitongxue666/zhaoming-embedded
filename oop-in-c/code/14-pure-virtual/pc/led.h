/* SPDX-License-Identifier: MIT */
/*
 * led.h - led_ops 字段集 + 父类统一接口 (ch14 版, 必填 + 选填混合策略)
 *
 * 这一份 led.h 在 ch14 退到"公开接口集中点": 应用层 #include 这一个
 * 头就拿到 struct led_ops + led_on / led_off / led_set_brightness +
 * struct led_base. 子类头文件 (led_gpio.h / led_pwm.h) 各自单独一份,
 * 只在认识硬件的位置 (main.c / board_init.c) include.
 *
 * 这一章在 ch13 的基础上给 ops 表加了 set_brightness 字段. 三个字段
 * 对应三种用法:
 *
 *     struct led_ops {
 *         int (*on)(struct led_base *me);                 // 必填
 *         int (*off)(struct led_base *me);                // 必填
 *         int (*set_brightness)(struct led_base *me,      // 选填
 *                               uint8_t brightness);
 *     };
 *
 * 必填走 assert (见 led_base.c led_on / led_off), 选填走父类默认行为
 * (见 led_base.c led_set_brightness). ops 表字段类型本身不变, "必填还
 * 是选填"这条纪律落在父类统一接口里. 子类填了走子类, 没填: 必填的崩,
 * 选填的走默认.
 *
 * GPIO 子类故意只填 on / off, 不填 set_brightness, 演示选填策略.
 * PWM 子类三件套全填.
 *
 * 子类实现里 (gpio_on / pwm_on / ...) 用 ch13 学的 container_of 反推
 * 子类指针, 强转那一招在 ch13 故意挪 base 演示后已经退役.
 *
 * 见 ch14 § 14.2 / 14.3.
 */

#ifndef LED_H
#define LED_H

#include "led_base.h"

/*
 * struct led_ops - 操作表.
 *
 * on / off 必填, set_brightness 选填. 三种字段类型完全一样, 区别只
 * 在父类统一接口里怎么处理 NULL.
 */
struct led_ops {
	int (*on)(struct led_base *me);                 /* 必填 */
	int (*off)(struct led_base *me);                /* 必填 */
	int (*set_brightness)(struct led_base *me,      /* 选填 */
	                      uint8_t brightness);
};

/* ============== 父类统一接口 (必填 + 选填) ==============
 *
 * 三个统一接口函数都接 base 指针. 函数体里走 me->ops->xxx(me), 那张
 * 表是跟着 me 自己跑的. 应用层不用知道下面挂的是哪种 LED.
 *
 * led_on / led_off    -> 必填 (assert)
 * led_set_brightness  -> 选填 (NULL 时父类默认行为)
 */
int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_set_brightness(struct led_base *me, uint8_t brightness);

#endif /* LED_H */
