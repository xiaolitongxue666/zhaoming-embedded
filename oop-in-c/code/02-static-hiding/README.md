# 02-static-hiding — 把字段藏起来

第 2 章 [同事改了一行 LED 全乱了](../../../book/01-封装/02-同事改了一行.md) 的配套代码。

## 三套实现

```
02-static-hiding/
├── pc/                 完整可跑的 PC 模拟版（gcc 一句编译）
├── stm32-snippet/      STM32 HAL 等效片段（不是完整工程，见附录 B）
└── linux-snippet/      Linux 用户态 sysfs 等效片段（不是完整工程，见附录 C）
```

## 教学要点

ch01 的 `struct led` 字段是公开的，外部 `me->pin = 999` 能直接把它
弄坏。ch02 把字段定义从 `led.h` 搬到 `led.c`，`led.h` 只留前向声明
`struct led;`，外部代码：

- 看得到 `struct led *` 这个指针类型
- 看不到字段
- 想 `me->pin = ...` 编译就过不去

字段名一字不动（`pin / brightness / is_on`），改的只是 visibility。

附带：`led.c` 里的 `update_hardware` / `brightness_valid` 加 `static`，
进一步关闭一道门——别的 .c 看不到这些工具函数，只能走 `led.h` 的菜单。

## 编译运行（PC 版）

```bash
cd pc
make
./demo
```

期待输出：两颗 LED 各做一遍 on/off/toggle/set_brightness/get_state，
out-of-range 亮度被 API 拒绝（返回 -2），最后清理。

## STM32 / Linux 完整工程

完整跑通的 STM32 工程见 [附录 B](../../../book/附录/B-STM32完整工程.md)，
完整跑通的 Linux 工程见 [附录 C](../../../book/附录/C-Linux完整工程.md)。
（这两个附录在阶段 5 完成。）
