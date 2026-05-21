# ch12 · 向上转型

## 在线阅读

[向上转型](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/12-向上转型.html)

## 学习目标

- `struct led_base *` 指向任意子类\n- 数组 + 循环多态调用

## 需手写文件（建议）

base 指针表 + 子类实例

## 验收

一个循环处理 gpio / pwm / i2c

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/12-upcasting/`](../../../../oop-in-c/code/12-upcasting/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
