/* SPDX-License-Identifier: MIT */
/*
 * platform_module_export.h - 8 级 initcall 注册机制
 *
 * 编译期通过 __attribute__((section)) 把每个注册项排进对应的 ELF 段,
 * 启动期 platform_module_export_exec() 顺序遍历 1-7 级跑完, 第 8 级
 * UNIT_TEST 单独由 platform_unit_test_exec() 跑.
 *
 *   1 BOARD       板级 (时钟 / GPIO clock)
 *   2 PREV        软件预初始化
 *   3 DEVICE      设备驱动 (uart / i2c / spi / pin)
 *   4 COMPONENT   组件 (log / shell / FS)
 *   5 ENV         环境配置 (LED 实例配置 / 设备绑定)
 *   6 APP         应用任务
 *   7 SYSTEM_READY 系统就绪
 *   8 UNIT_TEST   单元测试
 *
 * 真机版用 linker 段, PC mock 版用 GCC __attribute__((constructor(N)))
 * 在 main 之前自动跑.
 */

#ifndef PLATFORM_API_MODULE_EXPORT_H_
#define PLATFORM_API_MODULE_EXPORT_H_

#include <stdint.h>

struct module_export {
	void (*func)(void);
};

#ifdef MOCK_PC

/* PC mock: GCC ctor 在 main 之前自动跑 */
#define _PLATFORM_INIT_CTOR(_func, _prio)                            \
	__attribute__((constructor(_prio)))                              \
	static void _func##_module_init_ctor(void) { _func(); }

#define INIT_BOARD_EXPORT(fn)            _PLATFORM_INIT_CTOR(fn, 101)
#define INIT_PREV_EXPORT(fn)             _PLATFORM_INIT_CTOR(fn, 102)
#define INIT_DEVICE_EXPORT(fn)           _PLATFORM_INIT_CTOR(fn, 103)
#define INIT_COMPONENT_EXPORT(fn)        _PLATFORM_INIT_CTOR(fn, 104)
#define INIT_ENV_EXPORT(fn)              _PLATFORM_INIT_CTOR(fn, 105)
#define INIT_APP_EXPORT(fn)              _PLATFORM_INIT_CTOR(fn, 106)
#define INIT_SYSTEM_READY_EXPORT(fn)     _PLATFORM_INIT_CTOR(fn, 107)
#define UNIT_TEST_EXPORT(fn)             _PLATFORM_INIT_CTOR(fn, 108)

#else  /* ! MOCK_PC: 真机三大编译器 */

#ifndef MODULE_EXPORT_SECTION
#if defined(__CC_ARM) || defined(__CLANG_ARM)
#define MODULE_EXPORT_SECTION(x)         __attribute__((section(x)))
#elif defined(__IAR_SYSTEMS_ICC__)
#define MODULE_EXPORT_SECTION(x)         @ x
#elif defined(__GNUC__)
#define MODULE_EXPORT_SECTION(x)         __attribute__((section(x)))
#else
#define MODULE_EXPORT_SECTION(x)
#endif
#endif

#ifndef MODULE_EXPORT_USED
#if defined(__CC_ARM) || defined(__CLANG_ARM)
#define MODULE_EXPORT_USED               __attribute__((used))
#elif defined(__IAR_SYSTEMS_ICC__)
#define MODULE_EXPORT_USED               __root
#elif defined(__GNUC__)
#define MODULE_EXPORT_USED               __attribute__((used))
#else
#define MODULE_EXPORT_USED
#endif
#endif

#define INIT_EXPORT(_func, level)                                   \
	MODULE_EXPORT_USED const struct module_export                   \
	module_init_##_func MODULE_EXPORT_SECTION("moduleExport" level) = \
	{                                                               \
		.func = _func,                                              \
	}

#define INIT_BOARD_EXPORT(fn)            INIT_EXPORT(fn, "1")
#define INIT_PREV_EXPORT(fn)             INIT_EXPORT(fn, "2")
#define INIT_DEVICE_EXPORT(fn)           INIT_EXPORT(fn, "3")
#define INIT_COMPONENT_EXPORT(fn)        INIT_EXPORT(fn, "4")
#define INIT_ENV_EXPORT(fn)              INIT_EXPORT(fn, "5")
#define INIT_APP_EXPORT(fn)              INIT_EXPORT(fn, "6")
#define INIT_SYSTEM_READY_EXPORT(fn)     INIT_EXPORT(fn, "7")
#define UNIT_TEST_EXPORT(fn)             INIT_EXPORT(fn, "8")

#endif /* MOCK_PC */

extern void platform_module_export_exec(void);
extern void platform_unit_test_exec(void);

#endif /* PLATFORM_API_MODULE_EXPORT_H_ */
