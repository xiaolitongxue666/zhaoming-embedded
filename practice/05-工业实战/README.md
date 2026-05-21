# 第五部分 · 开源工程实战（ch19–ch20）

`practice/` 在 ch18 之后**不再提供** `pc/` 骨架。工业实战沿用仓库现有 Zephyr / Linux 工程，与在线书第五部分、附录 B/C 配套。

## ch19 · Zephyr 实战

**阅读**：[用前 18 章的眼睛读 driver subsystem](https://zhaochengbo.github.io/zhaoming-embedded/05-工业实战/19-主控案例.html)

**工程**：[`industrial-zephyr/`](../../industrial-zephyr/)

```bash
cd industrial-zephyr
west build -b stm32f4_disco -p auto -- -DDEMO=1
west flash
```

`DEMO=1/2/3/4` 对应书中四个 demo（4 LED / overlay / CONTAINER_OF / 可空 ops）。

**附录**：[附录 B · STM32 完整工程](https://zhaochengbo.github.io/zhaoming-embedded/附录/B-STM32完整工程.html)

### 无开发板时

- 阅读 `industrial-zephyr/src/main_demo*.c` 与 `README.md`
- 对照 Zephyr 上游 driver 子系统源码（书中 permalink）
- 回到 `oop-in-c/code/13-container-of` 等 PC demo 巩固概念

---

## ch20 · Linux 实战

**阅读**：[写一个自己的内核驱动](https://zhaochengbo.github.io/zhaoming-embedded/05-工业实战/20-子控案例.html)

**工程**：[`industrial-linux/`](../../industrial-linux/)

```bash
cd industrial-linux/ch20-leds-status
make
sudo insmod leds-status.ko
```

子目录还包括 libgpiod 对照、QEMU+gdb、ftrace initcall 追踪。

**附录**：[附录 C · Linux 完整工程](https://zhaochengbo.github.io/zhaoming-embedded/附录/C-Linux完整工程.html)

### 无树莓派时

- 精读 `industrial-linux/ch20-leds-status/leds-status.c`
- 用 QEMU 子目录（见附录 C）或仅读源码理解 sysfs / platform_driver

---

## 进度

完成阅读与（可选）上板实验后，在 [`LEARNING_PLAN.md`](../LEARNING_PLAN.md) 勾选 ch19、ch20 及附录 B/C。

## 代码索引

[附录 D · 配套代码索引](https://zhaochengbo.github.io/zhaoming-embedded/附录/D-配套代码索引.html) · 仓库 [`book/附录/D-配套代码索引.md`](../../book/附录/D-配套代码索引.md)
