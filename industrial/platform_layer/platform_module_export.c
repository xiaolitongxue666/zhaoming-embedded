/**
  ******************************************************************************
  * @file    platform_module_export.c
  * @brief   8 级链接自动初始化机制 dispatcher 实现.
  *
  * @details 见第 17 章 "initcall". 三家编译器拿到段首/段尾符号的方式不一样,
  *          这里用 #if defined(__CC_ARM) / __ICCARM__ / __GNUC__ 三路分支适配:
  *            - ARMCC: 由链接器自动生成 sectionName$$Base / $$Limit 符号
  *            - IAR:   #pragma section + __section_begin/end
  *            - GCC:   linker script 自定义 moduleExport<i>_start/_end 符号
  *          得到段首段尾后, 整段当成 module_export_t 数组遍历, 调每个 func.
  *
  *          MODULE_EXPORT_EXEC(N) 宏统一这三种方式的差异, 上层
  *          platform_module_export_exec() 顺序展开 1-7 级即可.
  ******************************************************************************
  */

#include "platform_module_export.h"

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
#define MODULE_EXPORT_EXEC(section)\
    do\
    {\
        module_export_t *p_module_export_base;\
        uint32_t module_export_count;\
        p_module_export_base = (module_export_t *)(&section##$$Base);\
        module_export_count = ((unsigned int)(&section##$$Limit)\
                                - (unsigned int)(&section##$$Base))\
                                / sizeof(module_export_t);\
        for(uint32_t i = 0; i < module_export_count; i++)\
        {\
            p_module_export_base[i].func();\
        }\
    }while(0)

#elif defined(__ICCARM__) || defined(__ICCRX__)
#define MODULE_EXPORT_EXEC(section)\
    do\
    {\
        module_export_t *p_module_export_base;\
        uint32_t module_export_count;\
        p_module_export_base = (module_export_t *)(__section_begin(#section));\
        module_export_count = ((unsigned int)(__section_end(#section))\
                                - (unsigned int)(__section_begin(#section)))\
                                / sizeof(module_export_t);\
        for(uint32_t i = 0; i < module_export_count; i++)\
        {\
            p_module_export_base[i].func();\
        }\
    }while(0)

#elif defined(__GNUC__)
#define MODULE_EXPORT_EXEC(section)\
    do\
    {\
        module_export_t *p_module_export_base;\
        uint32_t module_export_count;\
        p_module_export_base = (module_export_t *)(&moduleExport##section##_start);\
        module_export_count = ((unsigned int)(&moduleExport##section##_end)\
                                - (unsigned int)(&moduleExport##section##_start))\
                                / sizeof(module_export_t);\
        for(uint32_t i = 0; i < module_export_count; i++)\
        {\
            p_module_export_base[i].func();\
        }\
    }while(0)
#endif

void platform_module_export_exec()
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



