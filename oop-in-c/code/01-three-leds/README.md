# 01-three-leds — 三颗 LED 三份代码

第 1 章 [三个 LED 三份代码](../../../book/01-封装/01-三个LED三份代码.md) 的配套代码。

## 三套实现

```
01-three-leds/
├── pc/                 完整可跑的 PC 模拟版（gcc 一句编译）
├── stm32-snippet/      STM32 HAL 等效片段（不是完整工程，见附录 B）
└── linux-snippet/      Linux 用户态 sysfs 等效片段（不是完整工程，见附录 C）
```

## 教学要点

三套实现共享同一份 `led.h` 和 `led.c` ——
`struct led` + `me` 指针的封装。
变化的只是 `platform_*` 这层胶水。

这是平台抽象层最直接的威力，也是为什么 Linux 内核 4000 万行代码能跨几十种 CPU 架构跑的核心机制。

## 编译运行（PC 版）

```bash
cd pc
make
./demo
```

期待输出：三颗 LED（Pin 13/14/15）依次被同一份 `led_on()` 点亮，传不同的 `&red_led / &green_led / &blue_led` 指针即可。

## STM32 / Linux 完整工程

完整跑通的 STM32 工程见 [附录 B](../../../book/附录/B-STM32完整工程.md)，
完整跑通的 Linux 工程见 [附录 C](../../../book/附录/C-Linux完整工程.md)。
（这两个附录在阶段 5 完成。）
