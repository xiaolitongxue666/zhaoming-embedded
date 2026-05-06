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
 * 只调存好的函数指针. 应用层是被动等待 driver 调过来, 而不是主动
 * 询问 driver, 这是分层解耦的常见做法.
 *
 * typedef led_state_cb 给函数指针类型起短名 (见 ch08 § 8.7 衍生场景
 * 一节). 函数指针 typedef 是少数 Linus 也支持的 typedef 例外, 因为
 * 类型字面量太长, 起短名纯收益. typedef 的细节展开见 ch09 § 9.2.
 *
 * 注: ch07 演示了独立函数指针变量 (没有把指针塞进 struct), ch08
 * 的焦点是"函数指针当参数 / 当回调字段", 子类形态和 ch06 一致.
 * ops 系统化要等 ch09/ch10.
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
	 * 工业代码里这个字段绝大多数在启动期一次填好就再不动 (避免
	 * 任务/中断并发改写带来的竞态).
	 */
	led_state_cb on_state_change;
};

int led_base_init(struct led_base *me, const char *name);
int led_register_state_cb(struct led_base *me, led_state_cb cb);
const char *led_base_get_name(const struct led_base *me);

#endif /* LED_BASE_H */
