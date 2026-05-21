# ch01 · 三个 LED 三份代码

## 在线阅读

[三个 LED 三份代码](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/01-三个LED三份代码.html)

## 学习目标

- 体会三份重复代码的痛点\n- `struct led` + `me` 指针\n- 通过 `platform_gpio_*` 访问硬件

## 需手写文件（建议）

`led.h`、`led.c`

## 验收

三颗 LED（Pin 13/14/15）依次点亮，终端打印 GPIO 模拟日志

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/01-three-leds/`](../../../../oop-in-c/code/01-three-leds/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
