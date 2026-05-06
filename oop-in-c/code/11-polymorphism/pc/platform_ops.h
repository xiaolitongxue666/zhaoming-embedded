/* SPDX-License-Identifier: MIT */
/**
 * @file  platform_ops.h
 * @brief platform 层内部 ops 表 - "OOP 机制递归用在平台层"
 *
 * @details
 * 同一个 ops 机制, ch10 用在设备层 (每颗 LED 自带 ops), ch11 再用
 * 一遍在平台层. 对照表见 ch11 § 11.5:
 *   ch10 设备层                          | ch11 平台层
 *   -------------------------------------+--------------------------------
 *   struct led_base { const ops *ops; }  | struct platform_ops 多份实例
 *   子类 init 把 &led_ops_xxx 填进 base  | platform_select(&platform_xxx)
 *   led_on 内部 me->ops->on(me)          | platform_gpio_write 内部 g_ops->...
 *   加新 LED 类型不改 led_on             | 加新平台不改 platform_gpio_write
 *
 * 这一份是 platform 层的内部细节, 对外不暴露给驱动层. 驱动层只调
 *   platform_gpio_init / platform_gpio_write / platform_gpio_read /
 *   platform_gpio_deinit
 * 这组封装函数 (在 common/platform.h 声明, 签名从 ch01 起一字不变).
 *
 * ch01 ~ ch10 这组封装函数是"直接执行 GPIO 操作"; ch11 起函数体改
 * 为"先 LDR 当前 ops 指针, 再 LDR ops->gpio_write, 跳过去执行".
 * 驱动层调用形态没变, 但下面跑的 platform 实现可以热切换.
 *
 * 为什么要这么演化 (见 ch11 § 11.5.1):
 *   1) 编译期决定平台 -> 启动期挑表: platform_select(&platform_xxx)
 *   2) 单元测试想 mock platform: 填一张测试用 ops 表, 直接切过去
 *   3) 一个进程同时模拟多种 platform (教学场景): runtime 切换
 *
 * 代价 (见 ch11 § 11.5.2):
 *   封装函数体多两次 LDR, ARM Cortex-M4 @ 168MHz 多 ~28ns. 主流
 *   嵌入式驱动调用频率 (秒级几十到几千次) 完全可以忽略. 极端
 *   高频 (PWM 微秒级) 应该走专用接口, 不上 ops 表.
 *
 * platform_ops.h 不放 common/, 这是 platform 层内部头. 驱动层
 * 只 include common/platform.h, 永远看不到这张 ops 表.
 */

#ifndef PLATFORM_OPS_H
#define PLATFORM_OPS_H

#include <stdint.h>
#include <stdbool.h>

/*
 * platform_ops - 把"一种平台能做的所有 GPIO 操作"打包成一张表.
 * 跟 struct led_ops 完全同构, 区别只是字段集对应不同抽象层.
 */
struct platform_ops {
	const char *name;
	void (*gpio_init)(uint8_t pin, uint8_t mode);
	void (*gpio_deinit)(uint8_t pin);
	void (*gpio_write)(uint8_t pin, bool value);
	bool (*gpio_read)(uint8_t pin);
};

/*
 * 具体实例声明 (在各自的 platform_ops_xxx.c 里定义).
 * 全部 const + extern, 落 .rodata, 全程序唯一一份.
 *
 * 这一份只声明 PC, ch11 章节里 STM32 / Linux 实例在 stm32-snippet/
 * 和 linux-snippet/ 里 (snippet 不参与 PC build). ch15 起会同时
 * 持有多张实例, runtime 切换演示同一份 firmware 跨平台.
 */
extern const struct platform_ops platform_pc;

/* 切换 platform 层内部当前 ops 指针 */
void platform_select(const struct platform_ops *p);

/* 当前 platform 名字 (给日志打印用) */
const char *platform_name(void);

#endif /* PLATFORM_OPS_H */
