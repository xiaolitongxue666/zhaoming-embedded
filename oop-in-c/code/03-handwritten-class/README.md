# 03-handwritten-class — 你用 C 手搓了一个 class

第 3 章 [你用 C 手搓了一个 class](../../../book/01-封装/03-手搓class.md) 的配套代码。

## 目录结构

```
03-handwritten-class/
├── pc/                 完整可跑的 PC 模拟版（gcc 一句编译）
└── platform-mcu/
    └── stm32/          STM32 真机版（用 PIN_NUM 编码）
```

Linux 用户态完整工程见附录 C。

## 教学要点

两个模块同一个套路：

| 项 | LED | Motor |
|---|---|---|
| 头文件 | `led.h` | `motor.h` |
| 实现 | `led.c` | `motor.c` |
| 类型 | `struct led` | `struct motor` |
| 函数前缀 | `led_xxx` | `motor_xxx` |
| 构造 | `led_init` | `motor_init` |
| 析构 | `led_deinit` | `motor_deinit` |
| 字段 | `pin / brightness / is_on / initialized` | `pin / pwm_duty / direction / state / initialized` |

前缀把命名冲突挡在编译期之前，`init/deinit` 把生命周期写进函数名，
不需要看注释也知道哪个先调。

## 编译运行（PC 版）

```bash
cd pc
make
./demo
```

## STM32 / Linux 完整工程

完整跑通的 STM32 工程见 [附录 B](../../../book/附录/B-STM32完整工程.md)，
完整跑通的 Linux 工程见 [附录 C](../../../book/附录/C-Linux完整工程.md)。
（这两个附录在阶段 5 完成。）
