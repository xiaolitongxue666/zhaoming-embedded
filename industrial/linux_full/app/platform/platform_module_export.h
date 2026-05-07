/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_module_export.h
  * @brief   Header file of multi-level init mechanism.
  *
  * 见第 17 章 "initcall" + 附录 C § C.5.
  *
  * 8 级初始化机制 (跟 RT-Thread INIT_xxx_EXPORT 同款). 编译期通过
  * __attribute__((section)) 把每个注册项排进对应的 ELF 段, 启动期
  * platform_module_export_exec() 顺序遍历 1-7 级跑完, 第 8 级 (UNIT_TEST)
  * 单独由 platform_unit_test_exec() 跑.
  *
  * Linux 用户态下没有 linker 段 -- 这里改用 GCC __attribute__((constructor(N))),
  * ELF loader 在 main 之前会按 N 升序自动跑完, 应用层调用形态完全一致.
  *
  * 跨编译器 (ARMCC / IAR / GCC) 都支持.
  ******************************************************************************
  */

#ifndef PLATFORM_API_MODULE_EXPORT_H_
#define PLATFORM_API_MODULE_EXPORT_H_

#include "stdint.h"

typedef struct
{
    void (*func)(void);
} module_export_t;

/* ============================================================ */
#ifdef MOCK_PC

/* ---- PC mock: 用 GCC ctor 直接在 main 之前注册自动调用 ---- */

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

/* ============================================================ */
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
            MODULE_EXPORT_USED const module_export_t                \
            module_init_##_func MODULE_EXPORT_SECTION("moduleExport" level) = \
            {                                                       \
                .func = _func,                                      \
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

/******************** END OF FILE ********************/
