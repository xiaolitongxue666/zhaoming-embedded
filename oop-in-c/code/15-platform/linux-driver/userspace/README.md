# linux-driver/userspace · Linux 用户态 LED 框架

ch15 父类 / 子类 / 板级 / 应用四层落到 Linux 用户态的样子。

## 关键差别

**这一档目录没有 platform 抽象层。**

`led_gpio.c` 直接调 libgpiod, `led_pwm.c` 直接 open / write sysfs PWM 节点, `led_i2c.c` 直接 open `/dev/i2c-N` + ioctl。Linux 内核 driver model + chardev + sysfs 已经把硬件抽得干干净净, 应用层再套一层 `platform_gpio_init` -> `gpiod_line_request_output` 就是过度封装, 没拦下任何变化, 反而多一层 indirection。

ch15 § 15.11 / § 15.15 / § 15.16 反复讲过这件事, 这一档目录是代码兑现层。

## 文件清单

```
linux-driver/userspace/
├── led_base.h, led_base.c    父类层 + ops 表 (OOP 抽象, 任何平台都该有)
├── led_gpio.h, led_gpio.c    子类一 GPIO  -> libgpiod
├── led_pwm.h,  led_pwm.c     子类二 PWM   -> sysfs /sys/class/pwm/
├── led_i2c.h,  led_i2c.c     子类三 I2C   -> /dev/i2c-N + ioctl
├── main.c                    应用层 + 板级 (板子简单, 直接写 main 里)
├── Makefile                  链接 -lgpiod, 带 check-syntax target
└── README.md
```

## 跑

依赖 `libgpiod-dev`:

```
sudo apt install libgpiod-dev      # Debian / Ubuntu / 树莓派 OS
sudo dnf install libgpiod-devel    # Fedora / RHEL
```

编译 + 真机跑 (需要 root 或加 gpio / i2c group):

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

`main.c` 里的引脚 / 总线编号按树莓派 4B 默认 device tree 写:

- GPIO: `/dev/gpiochip0` 上的 line 17 (BCM GPIO17)
- PWM: pwmchip0 通道 0 (要在 `/boot/config.txt` 加 `dtoverlay=pwm-2chan` 才出 PWM 节点)
- I2C: `/dev/i2c-1` 上 7-bit 地址 0x20 (典型 PCA9555 / TCA9535 IO expander)

换板子改这几个数即可, 子类骨架 / 父类接口 / 应用层 0 改动。

## 和 drivers/ + platform/ MCU 版的对照

打开 `drivers/led/led_gpio.c` (跨 MCU 共享版) 和这一档 `linux-driver/userspace/led_gpio.c`:

- MCU 版: `platform_pin_write(self->pin, ...)` -- 中间多一层自抽 platform 接口 + ops 表 dispatch 到 `platform/arch/<mcu>/pin_board.c`.
- Linux 用户态版: `gpiod_line_set_value(self->line, ...)` -- 直接调内核接口, 无中间层.

应用层句柄都是 `struct led_base *`, OOP 抽象一致。差别只在子类层: 平台没有 driver model 的, 自己抽一层 (MCU); 平台已经有 driver model 的, 直接用 (Linux)。
