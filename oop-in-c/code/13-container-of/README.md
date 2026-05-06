# ch13 · container_of 的地址魔法 · 向下转型

配套书章节：[`book/04-工程威力/13-container_of.md`](../../../book/04-工程威力/13-container_of.md)

## 看点

- `container_of(ptr, type, member)` 的 PC 友好实现（见 `pc/container_of.h`）
- GPIO 子类故意把 base 放到第二个位置（前面挡了一个 `magic` 字段），证明 container_of 与 base 在 struct 里的位置无关
- 跑出来打印 `offsetof(struct led_gpio, base) = 4`，但每次操作仍能正确还原 magic、pin、on_level

## 目录

- `pc/` — 主线 demo（PC 模拟，任何 C99 编译器跑通）
- `stm32-snippet/led_stm32.c` — STM32 等效片段（函数式包装教学简化版，HAL_GPIO_*）
- `linux-snippet/led_linux.c` — Linux 用户态等效片段（函数式包装教学简化版，sysfs）

ch11+ 的 ops 表式 platform 抽象在 ch15 重构展开。

## 跑一遍

```
cd pc
make
./demo
```
