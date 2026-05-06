/* SPDX-License-Identifier: MIT */
/*
 * container_of.h - Linux 内核 container_of 宏的最小可用版本
 *
 * 这个宏解决了一个本来无解的问题：子类实现函数（gpio_on、pwm_on 等）
 * 只收到一个父类指针 struct led_base *me，但它要操作的字段（pin、
 * channel...）藏在外层子类 struct 里，不在 base 里。怎么从"成员地址"
 * 反推回"外层 struct 起始地址"？答案就是 container_of。
 *
 * 编译期算偏移、运行时只剩一条 sub 指令，零运行时开销。这是 C 用
 * 编译期数学解决了 C++ 用 RTTI（dynamic_cast）解决的同一个问题。
 *
 * 完整版见 Linux 内核 include/linux/container_of.h（v6.6 LTS 第 18-23
 * 行）。本文件保留同样的三步语义、去掉 GCC 专属的类型检查
 * （__same_type / static_assert），便于 PC 上用任何 C99 编译器跑通。
 *
 * 见 ch13 § 13.4 把它串起来 + § 13.7 这个东西叫什么。
 */

#ifndef MY_CONTAINER_OF_H
#define MY_CONTAINER_OF_H

#include <stddef.h>     /* offsetof */

/**
 * container_of - 从成员指针反推外层 struct 起始地址
 * @ptr:    指向某个成员的指针
 * @type:   外层 struct 的类型
 * @member: 该成员在外层 struct 里的名字
 *
 * 三步：
 *   1. 把 ptr 转成字节指针（char *），让减法按字节算
 *      （int * 减 1 减的是 4 字节，必须先转字节指针）
 *   2. 减去 member 在 type 里的偏移（offsetof 编译期常量）
 *   3. 把结果转回 type *
 *
 * 注意：ptr 表达式不要带副作用（像 get_next() 这种调用）。这个最小版
 * 把 (ptr) 在宏里展开两次（char 转换一次、offsetof 类型推导那次不用），
 * 副作用会被多次执行。Linux 内核版用 statement expression + __mptr 局部
 * 变量先抓一次，避免重求值，本最小版没做。详见 ch13 § 13.8.3。
 */
#define container_of(ptr, type, member)					\
	((type *)((char *)(ptr) - offsetof(type, member)))

#endif /* MY_CONTAINER_OF_H */
