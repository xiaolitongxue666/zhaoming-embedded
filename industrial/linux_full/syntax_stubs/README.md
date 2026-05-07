# syntax_stubs/

占位头文件, 用于 `make check-syntax` target.

在没有装 `libgpiod-dev` 或者不在 Linux 上 (Windows / macOS, 想本地校对 .c 文件
语法或重构) 的环境下, 这些 stub 提供 `<gpiod.h>` / `<linux/i2c-dev.h>` /
`<sys/ioctl.h>` 的最小符号声明, 让 `gcc -fsyntax-only` 能跑过.

只是 syntax / include 路径检查. 不能链接成可执行文件. 真机上跑要装真正的
libgpiod-dev:

```
sudo apt install libgpiod-dev
make             # 用真正的 <gpiod.h>, syntax_stubs/ 不会被搜索
```

为什么不直接 mock 出整套 libgpiod? 因为这一份工程的目标就是"在 Linux 上直接
用内核暴露的接口", mock 出整套 libgpiod 等于又把抽象层套回去了. syntax_stubs
只是开发期校对工具, 不是 build 模式.
