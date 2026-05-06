# ch15 · 换硬件不改应用 · Platform 抽象到底

配套书章节：[`book/04-工程威力/15-Platform抽象.md`](../../../book/04-工程威力/15-Platform抽象.md)

## 看点

ch15 是 platform 层 ops 化的高潮。`pc/` 里同时编译进 3 个 `struct platform_ops` 实例：

- `platform_pc`（PC 模拟）
- `platform_stm32_mock`（STM32 假装版，PC 上为了演示用 printf 模拟 BSRR 写入）
- `platform_linux_mock`（Linux 假装版，PC 上用 printf 模拟 sysfs 写入）

`main.c` 跑三轮，运行时通过 `platform_select(...)` 切换 platform。同一份业务代码、同一份 led 驱动、同一份 board_init，三种 platform 下走到不同 GPIO 实现，应用层 0 改动。

真实 STM32 / Linux 工程的 platform_ops 实现在 `stm32-snippet/` 和 `linux-snippet/`。

## 跑

```
cd pc
make
./demo
```

输出会看到 PC / STM32 / LINUX 三段，应用层代码完全相同。
