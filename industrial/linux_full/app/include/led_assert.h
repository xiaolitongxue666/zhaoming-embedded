/* SPDX-License-Identifier: MIT */
/*
 * led_assert.h - 父类 dispatch 防御性 assert.
 *
 * led_base.c 里收拢 ops / 函数指针校验, 失败时打印现场后 abort() (LED_ASSERT_HALT=1)
 * 或仅打印 (LED_ASSERT_HALT=0). 比 C 标准 assert() 多打印当前函数名.
 */

#ifndef APP_INCLUDE_LED_ASSERT_H_
#define APP_INCLUDE_LED_ASSERT_H_

extern void led_assert_handler(
	const char *ex_string, const char *func, int line);

#define led_assert(EX)                                                   \
	if (!(EX)) {                                                         \
		led_assert_handler(#EX, __FUNCTION__, __LINE__);                 \
	}

#endif /* APP_INCLUDE_LED_ASSERT_H_ */
