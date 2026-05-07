# ch17 linux-driver · Linux 内核 initcall 真身

ch17 整章讲的就是 Linux 内核的 initcall 机制。Linux 端不另外贴用户态实战代码 (initcall 的"产品形态"就是内核启动那一套, 不是用户态进程的事)。"snippet" 就是内核源码本身 (Linux 内核 v6.6 LTS, 如何在本地获取见附录 D):

- `include/linux/init.h`
  第 268 行：`____define_initcall` 宏定义（`__attribute__((__section__(...)))`）
- `include/linux/init.h`
  第 282 行：`__define_initcall(fn, id)` 把驱动塞进 `.initcall<id>` 段
- `include/linux/init.h`
  第 311 行：`device_initcall(fn) = __define_initcall(fn, 6)`
- `include/asm-generic/vmlinux.lds.h`
  第 908 行：`INIT_CALLS_LEVEL` 链接脚本宏，把所有 `.initcallN.init` 段合并
- `init/main.c`
  第 1297 行：`do_initcalls` 遍历调用所有 initcall

读完 pc/ 版再去读这五个内核源文件，骨架完全一样。

用户态 Linux：构造函数（`__attribute__((constructor))`）也能做"启动期自动调"，
机制更简单，但同源，把函数指针塞进 `.init_array` 段，crt0 启动期遍历。
