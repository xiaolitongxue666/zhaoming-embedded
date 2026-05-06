# linux_full — Linux 用户态完整工程（工业级骨架）

对应附录 C。

完整的 Linux 用户态工程，跑通全书 ch01-ch17 所有 OOP 抽象。功能：板上 4 颗 LED 流水灯（GPIO17 / GPIO27 / GPIO22 / GPIO23 BCM 编号，对应树莓派物理引脚 11 / 13 / 15 / 16）。

工程组织跟 [`stm32_full/`](../stm32_full/) 严格对称——同一套 `app/platform/` `app/drivers/` `app/environment_cfg/` 分层、同一组 `platform_err_t` 错误码、同一个 8 级 initcall 机制、同一个 `platform_assert` 校验、同一个字符串 pin 名机制。**唯一变化的是 `app/platform/arch/board/` 下 pin 子类**：STM32 用 HAL，Linux 用 libgpiod / sysfs。

## 工业纪律

|  层 | 看得到的内容 | 看不到的内容 |
|---|---|---|
| **应用层** (`src/main.c` / `mock/main_pc.c`) | `led_base_t *` 句柄 | 子类完整类型、ops 表、平台细节 |
| **驱动层** (`app/drivers/led/`) | `led_gpio_t`、`platform_pin_xxx()` 封装函数 | `platform_pin_ops_t`、libgpiod / sysfs API |
| **平台层** (`app/platform/`、`app/platform/arch/board/`) | 所有 ops 表、libgpiod、sysfs 文件接口 | / |

跨边界的纪律：

- 驱动层 `#include "platform/platform_pin.h"` 调封装函数，永远不直接调 libgpiod 或写 sysfs 文件
- `pin_libgpiod.c` 和 `pin_sysfs.c` 通过 `platform_pin_register(&xxx_pin_ops)` 启动期填进 framework，应用代码切换 backend 不用动一行
- 字符串 pin 名（`"GPIO17"` / `"GPIO27"`）：调用方写 BCM 编号，看不到 libgpiod line 句柄，也看不到 sysfs 文件路径

## 工程结构

```
linux_full/
├── README.md                                # 本文件
├── Makefile                                 # 三模 build (libgpiod / sysfs / MOCK)
├── src/
│   └── main.c                               # 真机主程序 (流水灯, signal-aware)
├── app/                                     # 应用层
│   ├── project_config.h                     # PLATFORM_OS / PLATFORM_HEAP_ENABLE 等开关
│   ├── platform/                            # 平台抽象层 (跟 stm32_full 完全同源)
│   │   ├── platform_def.h                   # 跨编译器宏 + platform_err_t + container_of
│   │   ├── platform_pin.h / .c              # PIN 框架 (字符串名 + ops 分发)
│   │   ├── platform_assert.h / .c           # platform_assert + 默认 handler
│   │   ├── platform_module_export.h / .c    # 8 级 INIT_xxx_EXPORT 机制
│   │   └── arch/board/
│   │       ├── pin_libgpiod.c               # libgpiod 子类 (默认, 现代 Linux 推荐)
│   │       └── pin_sysfs.c                  # sysfs 子类 (退路, 老内核兼容)
│   ├── drivers/
│   │   └── led/
│   │       ├── led_base.h / .c              # 父类接口 + 三层 platform_assert dispatch
│   │       └── led_gpio.h / .c              # GPIO LED 子类
│   └── environment_cfg/
│       └── led_cfg.c                        # 4 颗 LED 实例 + INIT_ENV_EXPORT
└── mock/                                    # PC 模拟模式
    ├── main_pc.c                            # PC 主程序
    └── pin_board_pc.c                       # PC pin 子类 (printf 模拟)
```

注意 `app/drivers/led/`、`app/platform/platform_def.h`、`app/platform/platform_pin.[hc]`、`app/platform/platform_assert.[hc]`、`app/platform/platform_module_export.[hc]`、`app/project_config.h` 这几份跟 `stm32_full/` 字节级一致。同一套抽象，换平台只换 `arch/board/` 下子类。

## 三种 build 模式

### PC 模拟模式（不依赖 GPIO 硬件）

```bash
cd industrial/linux_full
make MOCK=1
./build/firmware-pc
```

预期输出（节选）：

```
=========================================
  linux_full PC mock: 4 LED running light
=========================================

--- step 0 ---
    [PC] GPIO17 <- LOW
    [PC] GPIO27 <- LOW
    [PC] GPIO22 <- LOW
    [PC] GPIO23 <- LOW
    [PC] GPIO17 <- HIGH
...
=========================================
  done (3 rounds)
=========================================
```

### libgpiod 模式（默认，现代 Linux 推荐）

```bash
sudo apt install libgpiod-dev    # Debian/Ubuntu
# 或 sudo dnf install libgpiod-devel  # Fedora/RHEL
make
sudo ./build/firmware
```

`/dev/gpiochip0` 默认用树莓派 4B 的 GPIO 控制器。其他 SBC 改 `app/platform/arch/board/pin_libgpiod.c` 里的 `GPIO_CHIP_DEV` 即可。

### sysfs 模式（退路）

```bash
make BACKEND=sysfs
sudo ./build/firmware
```

sysfs 接口在内核 4.8 后被官方标记为 deprecated，新工程优先用 libgpiod。这里保留 sysfs 是为了演示"切 backend 时上层一字不改"。

## 跨平台移植

板子换了改 `app/environment_cfg/led_cfg.c` 的 4 个 BCM 字符串：

- `"GPIO17"` → 改成你板上的 GPIO 编号
- `light_level` 参数 `true` (高电平点亮) / `false` (低电平点亮) 按硬件接法

其余文件一字不动。

控制器换了（不是 `gpiochip0`）改 `pin_libgpiod.c` 的 `GPIO_CHIP_DEV` 一行。

## 跟 stm32_full 的差异

| 维度 | stm32_full | linux_full |
|---|---|---|
| 启动 | startup_*.s + Reset_Handler + linker section initcall | ELF loader + GCC ctor initcall |
| pin 子类 | HAL_GPIO_xxx | libgpiod / sysfs |
| pin 字符串 | "PA.5" / "PD.12" (port-pin 编码) | "GPIO17" / "GPIO27" (BCM 编号) |
| 主循环延时 | busy-wait `for (volatile i ...)` | `sleep(1)` |
| 信号处理 | NVIC 中断 | SIGINT / SIGTERM `signal()` |
| Make 目标 | `make` + `make flash` | `make` + `sudo ./build/firmware` |

两套工程的 `led_base.[hc]` / `led_gpio.[hc]` / `platform_pin.[hc]` / `platform_module_export.[hc]` 等抽象层文件**完全一致**。这就是这本书想给你的 takeaway：换平台只动 platform 子类，应用层和驱动层一字不动。

## 法律声明

按 [MIT License](../../LICENSE) 发布。教学用代码，不构成任何工业设备或控制系统的可用驱动。
