/* SPDX-License-Identifier: MIT */
/*
 * initcall.h - 山寨 Linux 内核 initcall 机制
 *
 * 这一章解决的本来无解的问题：每加一个驱动文件就得改一次 main / start_kernel
 * 加一行 xxx_init() 调用，违反开闭原则（对扩展开放、对修改关闭）。
 * 答案：让驱动文件"自己注册自己"，main 永远不动。
 *
 * 核心三件套：
 *   1. __attribute__((section(...)))：把函数指针塞进特殊段
 *   2. 链接器自动把同名段在所有 .o 文件里合并、生成 __start_<sec> /
 *      __stop_<sec> 边界符号
 *   3. 启动代码遍历 __start 到 __stop，挨个调用每一项
 *
 * 编译期 + 链接器 + 启动代码三方配合，main.c 不需要 #include 任何驱动
 * 文件，驱动文件也不需要 main 知道它。"零修改主干"加新驱动的关键。
 *
 * 真实内核版定义在 include/linux/init.h 第 268 行起：
 *   #define ____define_initcall(fn, __unused, __name, __sec)   \
 *       static initcall_t __name __used                        \
 *           __attribute__((__section__(__sec))) = fn;
 *
 * 我们的 PC 版砍掉级别（内核分 8 级 pure/core/postcore/arch/subsys/fs/
 * device/late），就一个级别 "my_initcall"。
 *
 * 见 ch17 § 17.2 module_init 是个魔法吗 + § 17.3 宏展开真相。
 */

#ifndef INITCALL_H
#define INITCALL_H

/* initcall_t = "无参、返回 int 的函数指针" 的类型别名。
 * 所有用 MODULE_INIT 注册的函数都必须是这个签名。do_initcalls 遍历段
 * 时就是按 sizeof(initcall_t) 步长往前移，每一步取出一个函数指针调一次。
 */
typedef int (*initcall_t)(void);

/*
 * MODULE_INIT(fn)：把 fn 的地址塞进 my_initcall 段
 *
 * 展开成一条静态变量定义：
 *   static initcall_t __initcall_<fn>
 *       __attribute__((used, section("my_initcall"))) = fn;
 *
 *  - section("my_initcall")：GCC 扩展，告诉编译器"这个变量不要放到默认
 *    .data 段，放到 my_initcall 段"。每个驱动文件 #include 一次本头、
 *    用一次 MODULE_INIT，就在 my_initcall 段里追加一个函数指针。
 *  - used：告诉编译器，这个变量虽然没有任何代码 grep 得到、看似
 *    "死代码"，但请保留不要优化掉。链接器会通过段名找到它，编译器
 *    自己看不到这条引用链。
 *  - static：让 __initcall_<fn> 这个变量名只在文件作用域可见，避免
 *    多个驱动撞名。链接器仍然能通过段名收集所有同名段。
 *  - __initcall_##fn：用 ## 拼接生成唯一变量名，每个驱动一份。
 *
 * 链接器自动收集所有 my_initcall 段的内容到同一连续区域，并生成
 * __start_my_initcall / __stop_my_initcall 边界符号（见下面 extern）。
 * 内核里的 __used / __section 同源同义。
 *
 * 见 ch17 § 17.3 宏展开真相。
 */
#define MODULE_INIT(fn)							\
	static initcall_t __initcall_##fn				\
		__attribute__((used, section("my_initcall"))) = fn

/*
 * 启动代码用这两个标记找到段的边界，遍历区间 [__start_my_initcall,
 * __stop_my_initcall) 即可拿到所有用 MODULE_INIT 注册的函数指针。
 *
 * GCC + 现代链接器（GNU ld、LLD）在 ELF / PE 平台上会为合法 C 标识符
 * 段名（不以点开头）自动生成 __start_<sec> / __stop_<sec> 符号——
 * 这就是为什么这里段名用 "my_initcall" 而不是 ".my_initcall"，
 * 让编译工具链帮我们把边界符号自动生成出来，省掉手写链接脚本。
 *
 * Linux 内核里也用同源机制，不过内核自己写链接脚本生成
 * __initcall_start / __initcall_end，结合 8 级 .initcallN.init 段。
 *
 * 声明成数组（[]）而不是指针（*），是因为这两个符号本质上是"段的
 * 起始地址"，不是指向地址的指针。用 [] 让 fn < __stop_my_initcall
 * 这种比较能直接当地址比较用。
 */
extern initcall_t __start_my_initcall[];
extern initcall_t __stop_my_initcall[];

void do_initcalls(void);

#endif /* INITCALL_H */
