# ch16 linux-driver/userspace · 应用层 libgpiod 最小例

ch16 整章讲的是 Linux 内核 GPIO 子系统的骨架: `struct gpio_chip` + ops 表 + 多态 dispatch (16.5 节). pc/ 山寨了一份能跑的极简版。

这一档目录把同一件事落到 **Linux 用户态**: 应用层调 libgpiod, libgpiod 背后就是内核 gpio chardev / gpiolib (内核完整 driver model 已经做完). 你看到的就是"ch16 内核子系统"的用户态视角。

## 关键差别

**没有 platform 抽象层。**

`led_userspace.c` 直接 `gpiod_line_set_value(line, value)`, 一个函数搞定。Linux 内核已经把 gpio_chip 多态 dispatch 全部做完, 应用层再套一层 `platform_gpio_write -> gpiod_line_set_value` 就是过度封装, 没拦下任何变化, 反而多一层 indirection。

ch16 § 16.13 / § 16.14 反复讲过这件事, 这一档目录是代码兑现层。

## 不贴内核驱动版的原因

LED 这种通用外设, Linux 内核 mainline 已经有 `drivers/leds/leds-gpio.c` 标准内核驱动 (上千种板子用过的工业级版本). 这本书重写一份"教学用 LED 内核驱动"是过度演示: 读者真要写内核驱动直接读 mainline 那一份就够。

什么时候真要自己写 LED 内核驱动? 看 ch16 § 16.14 三步判断流程: 内核没有 + 高频中断 + us 级延迟 + 多进程并发 + 大吞吐, 几条命中再上内核层。99% 的工业 LED 走应用层。

## 文件清单

```
linux-driver/userspace/
├── led_userspace.c    libgpiod 最小例 (60 行可跑)
├── Makefile           链接 -lgpiod, 带 check-syntax target
└── README.md
```

## 跑

依赖 `libgpiod-dev`:

```
sudo apt install libgpiod-dev      # Debian / Ubuntu / 树莓派 OS
sudo dnf install libgpiod-devel    # Fedora / RHEL
```

编译 + 真机跑 (需要 root 或加 gpio group):

```
make
sudo ./demo
```

Windows / 没 libgpiod-dev 的环境只做 syntax check:

```
make check-syntax
```

借用 `industrial/linux_full/syntax_stubs/` 那套占位头跑 `-fsyntax-only`。

## 真机假设

`led_userspace.c` 里的引脚按树莓派 4B 默认 device tree 写:

- `/dev/gpiochip0` 上的 line 17 (BCM GPIO17)

换板子改 `CHIP_NAME` / `LINE_OFFSET` 两个常量即可。

## 和 pc/ 山寨内核版的对照

打开 pc/gpiolib.c 第 30 行附近 `gpiod_set_value` 实现: 内部 `desc->gc->set(...)` 多态 dispatch 到 vendor_a_set / vendor_b_set. 这就是 Linux 内核 `drivers/gpio/gpiolib.c` 第 3057 行的山寨版。

打开本目录 `led_userspace.c`: 应用层一行 `gpiod_line_set_value(line, 1)`, 跨进程 syscall 进内核, 内核里走的是同一套 `gc->set` dispatch (真身, 不是山寨). 寄存器最终落到 BCM2711 SoC 的 GPIO 控制器实现 `drivers/gpio/gpio-bcm2835.c`。

应用层一行调用, 内核里跑的就是 ch16 整章讲的多态 dispatch。读完 pc/ 山寨版再读这一档应用层版, 你会看到"原来教学版的 gc->set 真的就在跑产品上"。
