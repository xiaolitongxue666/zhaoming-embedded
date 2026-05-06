# ch16 · 为什么 Linux 一点都不难

配套书章节：[`book/04-工程威力/16-Linux不难.md`](../../../book/04-工程威力/16-Linux不难.md)

## 看点

`pc/` 山寨了一份 Linux 内核的 GPIO 子系统：

- `gpio_chip.h`：`struct gpio_chip` 简化版（真身在内核 `include/linux/gpio/driver.h` 第 415 行）
- `gpiolib.c`：`gpiod_set_value` 简化版（真身在内核 `drivers/gpio/gpiolib.c` 第 3245 行）
- `gpio_vendor_a.c`、`gpio_vendor_b.c`：两家不同 SoC 的 GPIO 控制器
- `leds_gpio.c`：内核 `drivers/leds/leds-gpio.c` 的山寨，一行 `gpiod_set_value`，跨芯片

跑出来你会看到：同一个 `gpiod_set_value(led_red, 1)` 调用，红灯走 vendorA 的 set 函数，绿灯走 vendorB 的 set 函数。这就是 Linux 内核 OOP 风格的核心机制。

## 跑

```
cd pc
make
./demo
```
