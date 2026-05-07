/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_module_export.h
  * @brief   8 级链接自动初始化机制对外接口.
  *
  * @details 见第 17 章 "initcall" + 第 19 章 § 19.3 "initcall 7 级". 工业代码
  *          里每个驱动 / 配置层 / 应用模块自带一行 INIT_xxx_EXPORT(fn), 启动期
  *          platform_module_export_exec() 按 1->7 级顺序自动跑完, 不需要在
  *          main 里手写一长串 led_init(); uart_init(); ... 跟 RT-Thread 同款,
  *          也跟 Linux 内核 module_init / .initcall 段是一回事.
  *
  *          7 级语义 (跟 RT-Thread 一致):
  *            1 BOARD       板级 (时钟 / GPIO 时钟使能 / 复位)
  *            2 PREV        纯软件预初始化 (heap / 缓冲池)
  *            3 DEVICE      设备驱动 (uart / i2c / spi / pin)
  *            4 COMPONENT   组件 (log / shell / FAL / 文件系统)
  *            5 ENV         环境配置 (LED 实例配置 / 设备绑定)
  *            6 APP         应用任务 (业务线程启动)
  *            7 SYSTEM_READY 系统就绪 (灯亮表示启动完毕)
  *          第 8 级 UNIT_TEST 单独由 platform_unit_test_exec() 跑.
  ******************************************************************************
  */
#ifndef PLATFORM_API_MODULE_EXPORT_H_
#define PLATFORM_API_MODULE_EXPORT_H_
#include "stdint.h"

/* 注册项的格式: 一个 void(*)(void) 函数指针. 编译期把这种对象塞进
 * 名字以 "moduleExport<级数>" 开头的 ELF 段, 启动期遍历整段调用. */
typedef struct
{
    void (*func)(void);
} module_export_t;

#ifndef MODULE_EXPORT_SECTION
#if defined(__CC_ARM) || defined(__CLANG_ARM)
#define MODULE_EXPORT_SECTION(x)                __attribute__((section(x)))
#elif defined (__IAR_SYSTEMS_ICC__)
#define MODULE_EXPORT_SECTION(x)                @ x
#elif defined(__GNUC__)
#define MODULE_EXPORT_SECTION(x)                __attribute__((section(x)))
#else
#define MODULE_EXPORT_SECTION(x)
#endif
#endif

#ifndef MODULE_EXPORT_USED
#if defined(__CC_ARM) || defined(__CLANG_ARM)
#define MODULE_EXPORT_USED                      __attribute__((used))
#elif defined (__IAR_SYSTEMS_ICC__)
#define MODULE_EXPORT_USED                      __root
#elif defined(__GNUC__)
#define MODULE_EXPORT_USED                      __attribute__((used))
#else
#define MODULE_EXPORT_USED
#endif
#endif

#define INIT_EXPORT(_func, level) \
            MODULE_EXPORT_USED const module_export_t \
            module_int##_func MODULE_EXPORT_SECTION("moduleExport" level) =  \
            { \
                .func = _func, \
            }

/* board init */
#define INIT_BOARD_EXPORT(fn)           INIT_EXPORT(fn, "1")
/* components pre-initialization (pure software initilization) */
#define INIT_PREV_EXPORT(fn)            INIT_EXPORT(fn, "2")
/* device initialization */
#define INIT_DEVICE_EXPORT(fn)          INIT_EXPORT(fn, "3")
/* components initialization */
#define INIT_COMPONENT_EXPORT(fn)       INIT_EXPORT(fn, "4")
/* environment initialization (mount disk, ...) */
#define INIT_ENV_EXPORT(fn)             INIT_EXPORT(fn, "5")
/* appliation initialization */
#define INIT_APP_EXPORT(fn)             INIT_EXPORT(fn, "6")
/* system is ready */
#define INIT_SYSTEM_READY_EXPORT(fn)    INIT_EXPORT(fn, "7")
/* unity section */
#define UNIT_TEST_EXPORT(fn)            INIT_EXPORT(fn, "8")


extern void platform_module_export_exec(void);
extern void platform_unit_test_exec(void);

#endif /* PLATFORM_API_MODULE_INIT_H_ */
