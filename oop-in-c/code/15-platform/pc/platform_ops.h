/* SPDX-License-Identifier: MIT */
/*
 * platform_ops.h - platform 层内部 ops 表
 *
 * 这一份是 platform 层的内部细节，对外不暴露给驱动层。驱动层只调
 *   platform_gpio_init / platform_gpio_write / platform_gpio_read
 * 这一组封装函数（在 common/platform.h 里声明）。
 *
 * 三个 platform_ops 实例（PC 模拟 / STM32 模拟 / Linux 模拟）共存于
 * 同一个二进制里，启动期或测试期通过 platform_select(&platform_xxx)
 * 切换内部 ops 指针。封装函数体内部走 ops dispatch，自动落到当前选定
 * 的实现。这就是 ch15 演示"运行时切换平台"的关键。
 *
 * 这是 ch11 之后 platform 层"内部"演化的最终形态。对外接口（封装函数
 * 的签名）从 ch01 起没动过——这就是工业代码里"对外稳定、对内可换"的
 * 标准做法：上层从第一天起依赖的 4 个函数签名一直没变，platform 层
 * 内部从最朴素的 printf 直接实现，演化到现在的 ops 分发，上层 0 改动。
 *
 * 见 ch15 § 15.1 四层架构 + § 15.5 platform 层 ops 化。
 */

#ifndef PLATFORM_OPS_H
#define PLATFORM_OPS_H

#include <stdint.h>
#include <stdbool.h>

struct platform_ops {
	const char *name;
	void (*gpio_init)(uint8_t pin, uint8_t mode);
	void (*gpio_write)(uint8_t pin, bool value);
	bool (*gpio_read)(uint8_t pin);
	void (*gpio_deinit)(uint8_t pin);
};

/* 三个具体实例（在各自的实现文件里定义） */
extern const struct platform_ops platform_pc;
extern const struct platform_ops platform_stm32_mock;
extern const struct platform_ops platform_linux_mock;

/* 切换内部 ops 实例 */
void platform_select(const struct platform_ops *p);

/* 当前 platform 名字（封装函数，给日志打印用） */
const char *platform_name(void);

#endif /* PLATFORM_OPS_H */
