/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_def.h
  * @brief   Header file of platform common types, defines and so on.
  *
  * 见第 13 章 "container_of" + 附录 B § B.5.
  *
  * 多编译器兼容的 attribute 宏 + platform_err_t + container_of / offsetof.
  * ARMCC / IAR / GCC 三种工具链都覆盖, 跟 RT-Thread / Linux 内核风格一致.
  *
  * container_of 不是 C 标准里的, 是 Linux 内核用 offsetof + GCC 语句表达式
  * 自己拼出来的宏. 见第 13 章详细推导.
  ******************************************************************************
  */

#ifndef PLATFORM_API_PLATFORM_DEF_H_
#define PLATFORM_API_PLATFORM_DEF_H_

typedef enum
{
    PLATFORM_EOK       =  0,    /**< 没错 */
    PLATFORM_ERROR     = -1,    /**< 通用错误 */
    PLATFORM_ETIMEOUT  = -2,    /**< 超时 */
    PLATFORM_EFULL     = -3,    /**< 资源已满 */
    PLATFORM_EEMPTY    = -4,    /**< 资源已空 */
    PLATFORM_ENOMEM    = -5,    /**< 内存不足 */
    PLATFORM_ENOSYS    = -6,    /**< 不支持 / 未实现 */
    PLATFORM_EBUSY     = -7,    /**< 忙 */
    PLATFORM_EIO       = -8,    /**< IO 错误 */
    PLATFORM_EINTR     = -9,    /**< 系统调用被中断 */
    PLATFORM_EINVAL    = -10    /**< 无效参数 */
} platform_err_t;

#ifndef offsetof
#define offsetof(TYPE, MEMBER)  ((unsigned long) &((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
/**
  * @brief  Cast a member of a structure out to the containing structure.
  * @param  ptr     Pointer to the member.
  * @param  type    Type of the container struct this is embedded in.
  * @param  member  Name of the member within the struct.
  */
#define container_of(ptr, type, member)                  \
    ({                                                   \
        void *__mptr = (void *)(ptr);                    \
        ((type *)(__mptr - offsetof(type, member)));     \
    })
#endif

/* ------ 跨编译器 attribute 宏 ------------------------------------------- */
#if defined(__CC_ARM) || defined(__CLANG_ARM)           /* ARM Compiler 5/6 */
    #include <stdarg.h>
    #define PLATFORM_SECTION(x)         __attribute__((section(x)))
    #define PLATFORM_UNUSED             __attribute__((unused))
    #define PLATFORM_USED               __attribute__((used))
    #define PLATFORM_ALIGN(n)           __attribute__((aligned(n)))
    #define PLATFORM_WEAK               __attribute__((weak))
    #define PLATFORM_INLINE             static __inline

#elif defined(__IAR_SYSTEMS_ICC__)                       /* IAR */
    #include <stdarg.h>
    #define PLATFORM_SECTION(x)         @ x
    #define PLATFORM_UNUSED
    #define PLATFORM_USED               __root
    #define PLATFORM_PRAGMA(x)          _Pragma(#x)
    #define PLATFORM_ALIGN(n)           PLATFORM_PRAGMA(data_alignment=n)
    #define PLATFORM_WEAK               __weak
    #define PLATFORM_INLINE             static inline

#elif defined(__GNUC__)                                  /* GCC */
    #include <stdarg.h>
    #define PLATFORM_SECTION(x)         __attribute__((section(x)))
    #define PLATFORM_UNUSED             __attribute__((unused))
    #define PLATFORM_USED               __attribute__((used))
    #define PLATFORM_ALIGN(n)           __attribute__((aligned(n)))
    #define PLATFORM_WEAK               __attribute__((weak))
    #define PLATFORM_INLINE             static __inline

#else
    #error not supported tool chain
#endif

#endif /* PLATFORM_API_PLATFORM_DEF_H_ */

/******************** END OF FILE ********************/
