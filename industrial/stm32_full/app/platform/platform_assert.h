/* SPDX-License-Identifier: MIT */
/*
 * platform_assert.h - Lightweight runtime assert with file / func / line info.
 */

#ifndef PLATFORM_API_PLATFORM_ASSERT_H_
#define PLATFORM_API_PLATFORM_ASSERT_H_

extern void platform_assert_handler(
	const char *ex_string, const char *func, int line);

#define platform_assert(EX)                                              \
	if (!(EX)) {                                                         \
		platform_assert_handler(#EX, __FUNCTION__, __LINE__);            \
	}

#endif /* PLATFORM_API_PLATFORM_ASSERT_H_ */
