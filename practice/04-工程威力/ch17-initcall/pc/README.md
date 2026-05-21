# ch17 · initcall

## 在线阅读

[initcall](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/17-initcall.html)

## 学习目标

- `MODULE_INIT` 宏\n- 遍历 init 段表

## 需手写文件（建议）

`initcall.h`、多个 `drv_*.c`

## 验收

main 不显式调用各驱动 init

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/17-initcall/`](../../../../oop-in-c/code/17-initcall/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
