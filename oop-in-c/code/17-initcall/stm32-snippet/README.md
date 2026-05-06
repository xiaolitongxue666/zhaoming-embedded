# ch17 STM32 snippet

ch17 的 PC 版直接利用 GCC 的 `__attribute__((section()))` + 链接器自动生成
`__start_my_initcall` / `__stop_my_initcall` 符号。同样的机制在 STM32
裸机上也能跑，但你得在链接脚本（`.ld`）里加几行：

```
SECTIONS
{
    .my_initcall : ALIGN(4)
    {
        __start_my_initcall = .;
        KEEP(*(my_initcall*))
        __stop_my_initcall = .;
    } > FLASH
}
```

`KEEP(*)` 防止 LTO 把"没人显式引用"的初始化函数裁掉。和 Linux 内核
`include/asm-generic/vmlinux.lds.h` 第 908 行 `INIT_CALLS_LEVEL` 同源。

之后驱动文件里写 `MODULE_INIT(my_init)`，主程序 main 里调一次
`do_initcalls()`，所有驱动自动挂上来。

RT-Thread / Zephyr 都有现成的 initcall 框架（RT-Thread 叫
`INIT_DEVICE_EXPORT(fn)`，Zephyr 叫 `SYS_INIT(fn, level, prio)`），机制
完全一样。
