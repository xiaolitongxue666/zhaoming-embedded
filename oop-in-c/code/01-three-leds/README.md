# 01-three-leds — 三颗 LED 三份代码

第 1 章 [三个 LED 三份代码](../../../book/01-封装/01-三个LED三份代码.md) 的配套代码。

## 目录结构

```
01-three-leds/
├── pc/                 PC 模拟版（gcc 一句编译，最简）
└── platform-mcu/
    └── stm32/          STM32 真机版（用 PIN_NUM 编码）
```

ch01-ch10 早期章节都按这两份对照组织（PC + STM32）。Linux 用户态完整工程见附录 C。

## 教学要点

两套实现共享同一份 `led.h` 和 `led.c`：`struct led` + `me` 指针的封装。变化的只是 `platform_*` 这层胶水。

这是平台抽象层最直接的威力，也是为什么 Linux 内核 4000 万行代码能跨几十种 CPU 架构跑的核心机制。

## PIN 编码

`platform-mcu/stm32/led_stm32.c` 用一个 `uint8_t pin` 同时表示 port 和 pin 号：

- 高 4 位 = port 索引（A=0、B=1、...、I=8）
- 低 4 位 = pin 号（0-15）

例：`PA.13 = 0x0D`、`PD.12 = 0x3C`、`PI.14 = 0x8E`。

这套编码和 `industrial/stm32_full/app/platform/arch/board/pin_board.c` 字节级一致。读者过渡到工业版只多一层「字符串名 → uint8_t」的解析。

## 编译运行（PC 版）

```bash
cd pc
make
./demo
```

期待输出：三颗 LED（Pin 13/14/15）依次被同一份 `led_on()` 点亮，传不同的 `&red_led / &green_led / &blue_led` 指针即可。

## 编译运行（STM32 真机版）

`platform-mcu/stm32/led_stm32.c` 是片段，需要套到 STM32CubeMX 工程里编译。完整跑通的 STM32 工程见 [附录 B](../../../book/附录/B-STM32完整工程.md)。

应用层把 `led_init` 第二个参数改成 PIN 编码即可：

```c
led_init(&red_led,   0x0D);   /* PA.13 */
led_init(&green_led, 0x0E);   /* PA.14 */
led_init(&blue_led,  0x0F);   /* PA.15 */
```

板子上 LED 接到 PD.12，那就传 `0x3C`。`led.c` 一行不动。

## Linux 用户态完整工程

完整跑通的 Linux 工程见 [附录 C](../../../book/附录/C-Linux完整工程.md)。
