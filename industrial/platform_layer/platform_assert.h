/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    platform_assert.h
  * @brief   轻量级运行时断言宏。失败时把表达式字面 / 函数名 / 行号送到
  *          platform_assert_handler 现场打印 + 死锁定位。工业代码里用来
  *          在父类层兜住"子类忘填 ops / 实例没初始化"这类硬错误，见
  *          v_motor_base.c / h_motor_base.c 里多层 platform_assert 用法
  *          (附录 B § B.3 也能看到)。
  ******************************************************************************
  */

#ifndef PLATFORM_API_PLATFORM_ASSERT_H_
#define PLATFORM_API_PLATFORM_ASSERT_H_

/* 由各 port 的 platform_assert.c 实现, 见同名 .c. */
extern void platform_assert_handler(const char *ex_string, const char *func, int line);

/**
 * @brief  失败时把表达式字符串 / 当前函数名 / 行号送进 handler.
 * @param  EX  任何能转 bool 的表达式. 失败 (EX == 0) 时触发.
 *
 * #EX 把表达式字面字符串化, __FUNCTION__ / __LINE__ 是编译器内置宏.
 */
#define platform_assert(EX)                                                         \
if (!(EX))                                                                    \
{                                                                             \
    platform_assert_handler(#EX, __FUNCTION__, __LINE__);                           \
}

#endif /* PLATFORM_API_PLATFORM_ASSERT_H_ */
