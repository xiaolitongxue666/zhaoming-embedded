# ch03 · 手搓 class

## 在线阅读

[手搓 class](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/03-手搓class.html)

## 学习目标

- `模块_动作` 命名避免链接冲突\n- 句柄 + 操作函数\n- LED 与 motor 同套路

## 需手写文件（建议）

`led.h/c`、`motor.h/c`

## 验收

`led_init` / `motor_init` 并列演示，无符号冲突

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/03-handwritten-class/`](../../../../oop-in-c/code/03-handwritten-class/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
