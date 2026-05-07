/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_module_export.c
  * @brief   Multi-level init dispatcher.
  *
  * 真机版: 7 级 INIT_xxx_EXPORT 注册项放进 linker 段, 这一份遍历 7 级.
  * 第 8 级 UNIT_TEST 单独由 platform_unit_test_exec() 跑.
  *
  * PC mock 模式 (MOCK_PC) 下 ctor 已经在 main 之前跑过, 这两个函数都退化为 nop.
  ******************************************************************************
  */

#include "platform/platform_module_export.h"

#ifdef MOCK_PC

void platform_module_export_exec(void)
{
    /* nop: ctor 在 main 之前已经跑过 */
}

void platform_unit_test_exec(void)
{
    /* nop */
}

#else /* 真机三大编译器 */

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
extern const unsigned int moduleExport1$$Base;
extern const unsigned int moduleExport1$$Limit;
extern const unsigned int moduleExport2$$Base;
extern const unsigned int moduleExport2$$Limit;
extern const unsigned int moduleExport3$$Base;
extern const unsigned int moduleExport3$$Limit;
extern const unsigned int moduleExport4$$Base;
extern const unsigned int moduleExport4$$Limit;
extern const unsigned int moduleExport5$$Base;
extern const unsigned int moduleExport5$$Limit;
extern const unsigned int moduleExport6$$Base;
extern const unsigned int moduleExport6$$Limit;
extern const unsigned int moduleExport7$$Base;
extern const unsigned int moduleExport7$$Limit;
extern const unsigned int moduleExport8$$Base;
extern const unsigned int moduleExport8$$Limit;
#elif defined(__ICCARM__) || defined(__ICCRX__)
#pragma section="moduleExport1"
#pragma section="moduleExport2"
#pragma section="moduleExport3"
#pragma section="moduleExport4"
#pragma section="moduleExport5"
#pragma section="moduleExport6"
#pragma section="moduleExport7"
#pragma section="moduleExport8"
#elif defined(__GNUC__)
extern const unsigned int moduleExport1_start;
extern const unsigned int moduleExport1_end;
extern const unsigned int moduleExport2_start;
extern const unsigned int moduleExport2_end;
extern const unsigned int moduleExport3_start;
extern const unsigned int moduleExport3_end;
extern const unsigned int moduleExport4_start;
extern const unsigned int moduleExport4_end;
extern const unsigned int moduleExport5_start;
extern const unsigned int moduleExport5_end;
extern const unsigned int moduleExport6_start;
extern const unsigned int moduleExport6_end;
extern const unsigned int moduleExport7_start;
extern const unsigned int moduleExport7_end;
extern const unsigned int moduleExport8_start;
extern const unsigned int moduleExport8_end;
#endif

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
#define MODULE_EXPORT_EXEC(section)                                           \
    do                                                                        \
    {                                                                         \
        module_export_t *p_module_export_base;                                \
        uint32_t module_export_count;                                         \
        p_module_export_base = (module_export_t *)(&section##$$Base);         \
        module_export_count = ((unsigned int)(&section##$$Limit)              \
                                - (unsigned int)(&section##$$Base))           \
                                / sizeof(module_export_t);                    \
        for(uint32_t i = 0; i < module_export_count; i++)                     \
        {                                                                     \
            p_module_export_base[i].func();                                   \
        }                                                                     \
    } while (0)

#elif defined(__ICCARM__) || defined(__ICCRX__)
#define MODULE_EXPORT_EXEC(section)                                           \
    do                                                                        \
    {                                                                         \
        module_export_t *p_module_export_base;                                \
        uint32_t module_export_count;                                         \
        p_module_export_base = (module_export_t *)(__section_begin(#section));\
        module_export_count = ((unsigned int)(__section_end(#section))        \
                                - (unsigned int)(__section_begin(#section)))  \
                                / sizeof(module_export_t);                    \
        for(uint32_t i = 0; i < module_export_count; i++)                     \
        {                                                                     \
            p_module_export_base[i].func();                                   \
        }                                                                     \
    } while (0)

#elif defined(__GNUC__)
#define MODULE_EXPORT_EXEC(section)                                           \
    do                                                                        \
    {                                                                         \
        module_export_t *p_module_export_base;                                \
        uint32_t module_export_count;                                         \
        p_module_export_base =                                                \
            (module_export_t *)(&moduleExport##section##_start);              \
        module_export_count =                                                 \
            ((unsigned int)(&moduleExport##section##_end)                     \
            - (unsigned int)(&moduleExport##section##_start))                 \
            / sizeof(module_export_t);                                        \
        for(uint32_t i = 0; i < module_export_count; i++)                     \
        {                                                                     \
            p_module_export_base[i].func();                                   \
        }                                                                     \
    } while (0)
#endif

void platform_module_export_exec(void)
{
    MODULE_EXPORT_EXEC(1);  /* INIT_BOARD_EXPORT */
    MODULE_EXPORT_EXEC(2);  /* INIT_PREV_EXPORT */
    MODULE_EXPORT_EXEC(3);  /* INIT_DEVICE_EXPORT */
    MODULE_EXPORT_EXEC(4);  /* INIT_COMPONENT_EXPORT */
    MODULE_EXPORT_EXEC(5);  /* INIT_ENV_EXPORT */
    MODULE_EXPORT_EXEC(6);  /* INIT_APP_EXPORT */
    MODULE_EXPORT_EXEC(7);  /* INIT_SYSTEM_READY_EXPORT */
}

void platform_unit_test_exec(void)
{
    MODULE_EXPORT_EXEC(8);
}

#endif /* MOCK_PC */

/******************** END OF FILE ********************/
