# 04-data-classification — 数据归位

第 4 章 [你的全局变量该死了](../../../book/01-封装/04-数据归位.md) 的配套代码。

## 三套实现

```
04-data-classification/
├── pc/                 完整可跑的 PC 模拟版（gcc 一句编译）
│   ├── led_bad.c       反面教材：5 个全局变量满天飞
│   ├── led.c           正面教材：数据归位完成形态 + 静态对象池
│   ├── main.c          Part 1 跑 bad，Part 2 跑 good
│   └── ...
├── stm32-snippet/      STM32 HAL 等效片段（不是完整工程，见附录 B）
└── linux-snippet/      Linux 用户态 sysfs 等效片段（不是完整工程，见附录 C）
```

## 教学要点

数据归位三步走：

| 数据类型 | 归位方法 | 例子 |
|---|---|---|
| 实例数据 | struct 字段 | `me->pin` / `me->brightness` |
| 模块共享数据 | static 变量 | `static unsigned int s_init_count` |
| 只读常量 | static const | `static const uint8_t MAX_BRIGHTNESS = 100` |

加上：

| 对象生命周期 | 方法 | 例子 |
|---|---|---|
| 静态对象池 | static struct[N] | `static struct led led_pool[8]` |

ch02 用 malloc 的 `led_create / led_destroy`，ch04 改成静态池
`led_acquire / led_release`。MCU 上零堆碎片、O(1) 分配。

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
