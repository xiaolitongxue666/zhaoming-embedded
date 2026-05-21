# ch04 · 数据三级分类

## 在线阅读

[数据三级分类](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/04-数据归位.html)

## 学习目标

- 实例 / 模块 / 全局数据归位\n- 减少滥用全局可变状态

## 需手写文件（建议）

`led.h`、`led.c`（及书中涉及的分类示例）

## 验收

LED 行为不由散落全局变量驱动

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/04-data-classification/`](../../../../oop-in-c/code/04-data-classification/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
