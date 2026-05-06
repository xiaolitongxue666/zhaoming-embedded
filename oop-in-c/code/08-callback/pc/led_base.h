/* SPDX-License-Identifier: MIT */
/**
 * @file  led_base.h
 * @brief LED 基类 + 回调字段 - 把控制权临时交还给上层模块
 *
 * @details
 * 本章核心问题 (见 ch08 § 8.1, § 8.3):
 *   ch07 把"调谁开灯"挂在子类字段里, 解决了"换实现". 但应用层想做
 *   一件相反的事: 每次 LED 状态变化时, 自动通知它一下 (打日志、发
 *   CAN、写统计). led.c 不应该 #include 应用层模块的头文件 -- 这是
 *   分层的红线. 怎么让 led.c 反过来通知应用层?
 *
 * 答案: 在 base 里预留一个函数指针字段 on_state_change. 应用层
 * 调 led_register_state_cb(...) 把自己的函数地址挂进来; led_on /
 * led_off 状态翻转后调一下这个字段. led.c 不知道应用层是谁, 它
 * 只调存好的函数指针. 这就是好莱坞原则 "Don't call us, we'll
 * call you".
 *
 * typedef led_state_cb 给函数指针类型起短名, 见 ch08 § 8.4. 这是
 * 少数 Linus 也支持的 typedef 例外 (类型字面量太长, 起短名纯收益).
 *
 * 注: ch07 子类里的 on_func 字段 (子类自己挂函数指针) 在 ch08 这一份
 * 教学版里没有保留. 本章焦点是"函数指针当参数 / 当回调字段", 子类
 * 简化回 ch06 形态. ops 系统化要等 ch09/ch10.
 */

#ifndef LED_BASE_H
#define LED_BASE_H

#include "platform.h"

struct led_base;

/*
 * 回调类型: 接受 me + 新状态, 返回 void.
 * 用 typedef 起短名, 之后字段声明、注册函数签名都能写成
 *     led_state_cb cb;
 * 不用每次都写 void (*)(struct led_base *, bool).
 */
typedef void (*led_state_cb)(struct led_base *me, bool new_state);

struct led_base {
	const char  *name;
	bool         is_on;
	/*
	 * 状态变化回调字段, 可为 NULL (NULL 表示没人监听).
	 * led_on / led_off 内部会先 NULL check 再调, 不允许跳到地址 0.
	 * 工业代码里这个字段绝大多数在启动期一次填好就再不动, 避免
	 * 任务/中断之间的 TOCTOU 竞态 (见 ch08 § 8.6.2.2).
	 */
	led_state_cb on_state_change;
};

int led_base_init(struct led_base *me, const char *name);
int led_register_state_cb(struct led_base *me, led_state_cb cb);
const char *led_base_get_name(const struct led_base *me);

#endif /* LED_BASE_H */
