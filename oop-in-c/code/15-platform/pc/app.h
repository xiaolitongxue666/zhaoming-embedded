/* SPDX-License-Identifier: MIT */
/*
 * app.h - 应用层对外接口 (三个业务函数声明)
 *
 * 应用层只声明业务动作, 签名里没有任何硬件类型. alarm_blink /
 * status_indicate / power_on_test 是"业务语义"层面的描述, 下层走
 * led_base 句柄到具体硬件. 换硬件方案这一份头永远不动.
 *
 * 见 ch15 § 15.5 应用层.
 */
#ifndef APP_H
#define APP_H

void alarm_blink(void);
void status_indicate(int err_code);
void power_on_test(void);

#endif
