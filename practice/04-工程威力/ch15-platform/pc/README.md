# ch15 · Platform 抽象

## 在线阅读

[Platform 抽象](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/15-Platform抽象.html)

## 学习目标

- 应用 / 驱动 / platform / 板级\n- 换平台只改底层

## 需手写文件（建议）

platform init + led 驱动 + main

## 验收

应用 .c 只调 led API 与 platform 初始化

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/15-platform/`](../../../../oop-in-c/code/15-platform/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
