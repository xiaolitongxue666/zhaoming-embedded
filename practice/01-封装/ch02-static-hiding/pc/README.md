# ch02 · 同事改了一行 · static 与信息隐藏

## 在线阅读

[同事改了一行 · static 与信息隐藏](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/02-同事改了一行.html)

## 学习目标

- 字段 `/* private */` 约定\n- 内部工具函数 `static`\n- 读状态走 `led_get_state` API

## 需手写文件（建议）

`led.h`、`led.c`（可基于 ch01 演进）

## 验收

外部不直接破坏内部 pin/状态；API 访问可观测

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/02-static-hiding/`](../../../../oop-in-c/code/02-static-hiding/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
