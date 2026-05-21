# ch16 · Linux 不难

## 在线阅读

[Linux 不难](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/16-Linux不难.html)

## 学习目标

- tab 缩进与内核命名习惯\n- device / gpiolib 风格

## 需手写文件（建议）

按书整理的 linux-style 源文件

## 验收

风格与 oop-in-c ch16 demo 一致

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/16-linux-style/`](../../../../oop-in-c/code/16-linux-style/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
