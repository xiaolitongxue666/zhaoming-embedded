# linux_full — Linux 用户态完整工程（内核已做完 platform，应用层直接用）

对应附录 C。

完整的 Linux 用户态工程，跑通全书 ch01-ch16 所有 OOP 抽象。功能：板上 4 颗 LED 演示 GPIO + PWM + I2C 三种子类混搭，应用层只看 `struct led_base *` 句柄，看不到子类完整类型，"换硬件不改应用"在三种维度同时演示。

跟 [`stm32_full/`](../stm32_full/) 的关键区别：**这个工程没有 `app/platform/` 抽象层**。子类直接调 libgpiod / sysfs PWM / i2c-dev，因为 Linux 内核已经把 platform 抽象做完了，应用层再套一层是过度封装。这一对比是这本书工程判断力教学的核心。

## 为什么没有 platform 层

| 抽象层级 | stm32_full（裸机） | linux_full（用户态） |
|---|---|---|
| 应用层 | 调 `led_on(handle)` | 调 `led_on(handle)` |
| OOP base + 子类 | 同款 | 同款 |
| platform 层 ops 表 | **必须自抽**（HAL 不是设备模型） | **禁自抽**（内核已做完） |
| 板级实现 | `pin_board.c`（调 HAL） | **不存在**（libgpiod 就是内核暴露给用户的 platform 接口） |
| 硬件层 | STM32 寄存器 | Linux 内核 driver model |

工程判断力：

- 裸机 / 简单 RTOS：必须自抽 platform 层
- 带 device subsystem 的 RTOS（Zephyr / RT-Thread / NuttX）：内核已抽好，应用层别再抽
- Linux 用户态：内核做完，直接 libgpiod / i2c-dev / sysfs PWM
- Linux 内核态驱动：用内核 driver model

OOP 抽象（`struct led_base + 多子类多态 + 设备句柄统一导出`）保留，它解决"应用层不知道下层硬件细节"。但 platform 抽象层只在没有现成设备模型的环境才有价值，**在 Linux 上是反工程**。

## 工程结构

```
linux_full/
├── README.md                              # 本文件
├── Makefile                               # 单一 build (依赖 libgpiod-dev) + check-syntax
├── src/
│   └── main.c                             # 显式 environment_init() / exit()
├── app/
│   ├── project_config.h                   # LED_ASSERT_HALT 等小开关
│   ├── include/
│   │   ├── led_errors.h                   # platform_err_t enum (4 个值, 对齐 POSIX errno)
│   │   └── led_assert.h                   # led_assert 宏
│   ├── drivers/
│   │   └── led/
│   │       ├── led_base.h / .c            # 父类 dispatch + assert handler
│   │       ├── led_gpio.h / .c            # 直接调 libgpiod
│   │       ├── led_pwm.h  / .c            # 直接写 /sys/class/pwm/
│   │       └── led_i2c.h  / .c            # 直接走 /dev/i2c-N + ioctl(I2C_SLAVE)
│   └── environment_cfg/
│       ├── environment_export.h           # 4 个句柄 + environment_init/exit
│       └── led_cfg.c                      # gpiod_chip_open + 3 子类装配
└── syntax_stubs/                          # 不在 Linux / 没装 libgpiod 时的 syntax check 占位
    ├── README.md
    ├── gpiod.h
    ├── linux/i2c-dev.h
    └── sys/ioctl.h
```

## 4 颗 LED 配置

`app/environment_cfg/led_cfg.c` 装配 4 颗 LED 实例：

| 句柄 | 子类 | 资源 | set_brightness 行为 |
|---|---|---|---|
| `led_status` | `struct led_gpio` | gpiochip0 line 17 (BCM 17)，高电平点亮 | no-op (走父类默认) |
| `led_dimmer` | `struct led_pwm`  | pwmchip0/pwm0，1 kHz                    | 真生效，duty 0-255 |
| `led_panel`  | `struct led_i2c`  | /dev/i2c-1，addr 0x3C，reg 0x00         | no-op |
| `led_alarm` | `struct led_gpio` | gpiochip0 line 22 (BCM 22)，高电平点亮 | no-op |

应用层 `#include "environment_cfg/environment_export.h"` 一次拿到全部，调用一致：

```c
led_on(led_status);                  /* GPIO 子类底下 gpiod_line_set_value(line, 1) */
led_on(led_dimmer);                  /* PWM 子类底下 write enable_fd "1" */
led_set_brightness(led_dimmer, 128); /* 父类 dispatch 到 PWM 子类生效 */
led_set_brightness(led_status, 128); /* 父类 dispatch, GPIO 子类未实现, no-op */
```

## 编译运行

### 真机 Linux（树莓派 4B / 香橙派 / 飞腾派 / RK3588 等）

```bash
sudo apt install libgpiod-dev      # Debian / Ubuntu / 树莓派 OS
# 或 sudo dnf install libgpiod-devel  # Fedora / RHEL
make
sudo ./build/firmware
```

3 圈演示后退出，0 警告 0 错误。

### 不在 Linux 上 / 没装 libgpiod-dev

只能跑 syntax check，验证 .c 文件语法和头文件依赖：

```bash
make check-syntax
```

用 `syntax_stubs/` 下的占位 `<gpiod.h>` / `<linux/i2c-dev.h>` / `<sys/ioctl.h>` 让 `gcc -fsyntax-only` 跑过。**只是开发期校对工具**，不能链接成可执行文件。

## 跨 SBC 移植

板子换了改 `app/environment_cfg/led_cfg.c` 顶部的常量：

```c
#define LED_GPIO_CHIP_NAME    "gpiochip0"   /* gpiodetect 命令查 */
#define LED_STATUS_LINE       17             /* BCM 编号 */
#define LED_ALARM_LINE        22
#define LED_PWM_CHIP          0              /* /sys/class/pwm/pwmchip0 */
#define LED_PWM_NUM           0
#define LED_I2C_BUS           1              /* /dev/i2c-1 */
#define LED_I2C_ADDR          0x3C
```

其他文件一字不动。

## 已知细节

- 真机要 sudo（或者用户加 `gpio` / `i2c` group + 配 udev 规则）
- 树莓派要在 `/boot/config.txt` 加 `dtoverlay=pwm` 启用硬件 PWM、加 `dtparam=i2c_arm=on` 启用 I2C
- libgpiod 1.x vs 2.x API 改了：本工程用 1.x（Debian 12 / Ubuntu 22.04 默认）
- 完整对比看附录 C `book/附录/C-Linux完整工程.md`
