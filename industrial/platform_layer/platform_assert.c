/**
  ******************************************************************************
  * @file    platform_assert.c
  * @brief   默认 platform_assert handler 实现：printf 现场 + 死循环停在
  *          出错位置。真机版可重写为写日志 + 触发 watchdog reset + 推送
  *          告警。volatile 保证编译器不把 while 循环优化掉，出问题时
  *          调试器可以直接停在这一行回溯调用栈。
  ******************************************************************************
  */
#include <stdio.h>
#include <stdint.h>

/**
 * @brief  Default platform_assert handler.
 * @param  ex_string  失败的表达式字面 (#EX 字符串化得到).
 * @param  func       __FUNCTION__: 出错处的函数名.
 * @param  line       __LINE__: 出错处的行号.
 */
void platform_assert_handler(const char *ex_string, const char *func, int line)
{
	volatile char dummy = 0;
	printf("(%s) assertion failed at function:%s, line number:%d \n", ex_string, func, line);
	while (dummy == 0);   /* 故意死循环, 调试器在这里抓现场 */

}
