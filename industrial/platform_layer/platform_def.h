/**
  ******************************************************************************
  * @file    platform_def.h
  * @brief   Header file of platform common types, defines and so on.
  *
  * @details Platform 抽象层最底层公共头. 提供:
  *            - platform_err_t: 全局错误码 (一律负数, 0 = OK)
  *            - offsetof / container_of: 第 13 章核心宏, Linux 内核标志技术
  *            - 跨编译器 attribute 宏: ARMCC / IAR / GCC 三家全覆盖
  *          其他所有 platform_*.h 都直接或间接 include 这个头.
  ******************************************************************************
  */

#ifndef PLATFORM_API_PLATFORM_DEF_H_
#define PLATFORM_API_PLATFORM_DEF_H_

typedef enum
{
    PLATFORM_EOK = 0,			/**< There is no error */
    PLATFORM_ERROR = -1,		/**< A generic error happens */
    PLATFORM_ETIMEOUT = -2,		/**< Timed out */
    PLATFORM_EFULL = -3,		/**< The resource is full */
    PLATFORM_EEMPTY = -4,		/**< The resource is empty */
    PLATFORM_ENOMEM = -5,		/**< No memory */
    PLATFORM_ENOSYS = -6,		/**< No system */
    PLATFORM_EBUSY = -7,		/**< Busy */
    PLATFORM_EIO = -8,			/**< IO error */
    PLATFORM_EINTR = -9,		/**< Interrupted system call */
    PLATFORM_EINVAL = -10		/**< Invalid argument */
} platform_err_t;

#define typecheck(type,x) \
({  \
    type __dummy; \
    typeof(x) __dummy2; \
    (void)(&__dummy == &__dummy2); \
    1; \
})

#ifndef offsetof
#define offsetof(TYPE, MEMBER)  ((unsigned long)&((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 * 见第 13 章 "container_of". 这是 Linux 内核里出现频率最高的宏之一, 给定
 * 结构体内某个成员的指针 ptr, 反推出包含它的整个结构体的指针. 实现就是
 * 用 offsetof 量出成员相对于结构体首址的字节偏移, 把 ptr 减回去.
 *
 * 注: container_of 不在 C 标准里, 是 Linux 内核自己用 offsetof + GCC 语句
 * 表达式 ({ ... }) 拼出来的宏, 工业代码里照搬即可.
 */
#define container_of(ptr, type, member)\
	({\
		void *__mptr = (void *)(ptr);\
		((type *)(__mptr - offsetof(type, member)));\
	})
#endif

/* Compiler Related Definitions */
#if defined(__CC_ARM) || defined(__CLANG_ARM) /* ARM Compiler */
    #include <stdarg.h>
    #define PLATFORM_SECTION(x)               __attribute__((section(x)))
    #define PLATFORM_UNUSED                   __attribute__((unused))
    #define PLATFORM_USED                     __attribute__((used))
    #define PLATFORM_ALIGN(n)                 __attribute__((aligned(n)))
    #define PLATFORM_WEAK                     __attribute__((weak))
    #define PLATFORM_INLINE                   static __inline

#elif defined (__IAR_SYSTEMS_ICC__)           /* for IAR Compiler */
    #include <stdarg.h>
    #define PLATFORM_SECTION(x)               @ x
    #define PLATFORM_UNUSED
    #define PLATFORM_USED                     __root
    #define PLATFORM_PRAGMA(x)                _Pragma(#x)
    #define PLATFORM_ALIGN(n)                 PLATFORM_PRAGMA(data_alignment=n)
    #define PLATFORM_WEAK                     __weak
    #define PLATFORM_INLINE                   static inline

#elif defined (__GNUC__)                      /* GNU GCC Compiler */
    #include <stdarg.h>
    #define PLATFORM_SECTION(x)               __attribute__((section(x)))
    #define PLATFORM_UNUSED                   __attribute__((unused))
    #define PLATFORM_USED                     __attribute__((used))
    #define PLATFORM_ALIGN(n)                 __attribute__((aligned(n)))
    #define PLATFORM_WEAK                     __attribute__((weak))
    #define PLATFORM_INLINE                   static __inline
#else
    #error not supported tool chain
#endif

#endif /* PLATFORM_API_PLATFORM_DEF_H_ */

/******************** END OF FILE ******************END OF FILE****/
