/* SPDX-License-Identifier: MIT */
/*
 * main.h - Header for main.c (CubeMX 风骨架).
 *
 * 真机版用户从 CubeMX 把这一份替换为生成版即可. 本骨架只暴露 HAL 入口
 * 让 mock / 教学场景能 compile.
 */

#ifndef CORE_INC_MAIN_H_
#define CORE_INC_MAIN_H_

#ifndef MOCK_PC
#include "stm32f4xx_hal.h"
#endif

#endif /* CORE_INC_MAIN_H_ */
