# ch17 · 4000 万行一招写完 · 链接自动初始化

配套书章节：[`book/04-工程威力/17-initcall.md`](../../../book/04-工程威力/17-initcall.md)

## 看点

`pc/` 山寨了 Linux 内核 initcall 机制：

- `MODULE_INIT(fn)` 宏把驱动 init 函数地址塞进 `my_initcall` 段
- 4 个驱动文件（drv_led / drv_uart / drv_i2c / drv_spi）各自注册自己
- main 函数里没有显式调用任何 `*_init`
- 启动时 `do_initcalls()` 遍历段，挨个调用

## 跑

```
cd pc
make
./demo
```

输出会列出 4 个驱动 init 都跑到了，main 一行不动。
