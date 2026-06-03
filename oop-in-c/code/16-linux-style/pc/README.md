# ch16 Linux-style GPIO Subsystem

模拟 Linux 内核 GPIO 子系统的教学示例，来自《嵌入式 C 语言自我修养——从芯片、编译到 Linux 驱动》第 16 章。

## 五层架构层级关系

```
┌─────────────────────────────────────────────────────────────────┐
│  第5层  初始化/演示层  (main.c)                                  │
│  模拟内核启动 → probe → 获取 desc → 操作                        │
├─────────────────────────────────────────────────────────────────┤
│  第4层  设备驱动层    (leds_gpio.c)                              │
│  leds-gpio 驱动，调 gpiod_set_value，不关心底层是哪家芯片        │
├─────────────────────────────────────────────────────────────────┤
│  第3层  核心框架层    (gpiolib.c)                                │
│  注册 chip 管理列表 + consumer API                              │
│  gpiod_set_value → desc->gc->set 多态分发                       │
├─────────────────────────────────────────────────────────────────┤
│  第2层  芯片抽象接口  (gpio_chip.h)                              │
│  定义 struct gpio_chip（5 个函数指针 ops 表）                   │
│  定义 struct gpio_desc（chip + offset 对）                      │
├─────────────────────────────────────────────────────────────────┤
│  第1层  硬件实现层                                               │
│  ├─ gpio_vendor_a.c  (DR_REG 数据寄存器风格，片内 GPIO)         │
│  └─ gpio_vendor_b.c  (SET/CLR 寄存器风格，外扩 IO 扩展芯片)     │
└─────────────────────────────────────────────────────────────────┘
```

## 核心设计思想

### 1. 多态 dispatch

`gpiod_set_value` 内部只有一行：

```c
desc->gc->set(desc->gc, desc->offset, value);
```

根据 `desc->gc` 指向不同 chip 实例，走到不同厂商的实现：

- `desc->gc = &vendor_a_chip` → `vendor_a_set()` (DR_REG 风格)
- `desc->gc = &vendor_b_chip` → `vendor_b_set()` (BSRR 风格)

这是 C 语言面向对象编程的经典形态：`struct gpio_chip` 相当于虚基类，5 个函数指针相当于虚函数表，`desc->gc->set` 相当于虚函数调用。

### 2. N × M → N + M（中间层解耦）

```
                   LED 驱动   按键驱动   LCD 驱动
                      │          │          │
   ┌──────────────────┼──────────┼──────────┼───────────────┐
   │            gpiolib 抽象层 (gpio_chip + gpiod_*)       │
   └──────────────────┼──────────┼──────────┼───────────────┘
                      │          │          │
                ┌─────┴────┐ ┌──┴───┐ ┌───┴────┐
                │ vendorA  │ │ vendorB │ │ vendorC │
                │ (片内)    │ │ (外扩)   │ │ (I2C)   │
                └──────────┘ └────────┘ └─────────┘
```

- **没有抽象层：** M 个设备驱动 × N 家芯片 = M × N 份代码
- **有抽象层：** M 个设备驱动 + N 家芯片驱动 = M + N 份代码

芯片越多、设备越多，差值越大。这就是分层架构的核心收益。

### 3. 每层只做一件事

| 层 | 视角 | 关心什么 | 不关心什么 |
|----|------|---------|-----------|
| 第5层 main.c | 系统集成者 | 有哪些芯片、有哪些设备 | 寄存器布局 |
| 第4层 leds_gpio.c | 设备驱动工程师 | 调 gpiod_set_value 控 LED | 底层芯片型号、寄存器操作 |
| 第3层 gpiolib.c | 内核子系统维护者 | chip 列表管理、dispatch 接线 | 具体硬件实现 |
| 第2层 gpio_chip.h | 接口设计者 | 定义数据结构与契约 | 具体实现 |
| 第1层 vendor_*.c | 芯片厂商驱动工程师 | 芯片手册规定的寄存器操作 | 上层设备驱动逻辑 |

## 调用链全景

以点亮红色 LED（vendorA 第 5 脚）为例：

```
main.c               leds_gpio.c          gpiolib.c            gpio_vendor_a.c
  │                     │                    │                      │
  ├─ vendor_a_probe() ──┼────────────────────┼─ gpiochip_add() ────┤
  │                     │                    │  注册 vendor_a_chip  │
  │                     │                    │                      │
  ├─ gpio_get_desc() ───┼────────────────────┼─ 遍历 s_chips[]      │
  │  ("vendorA-gpio",5)  │                    │  → 匹配 label        │
  │                     │                    │  → 分配 desc          │
  │                     │                    │  → desc->gc = &vendorA│
  │                     │                    │                      │
  ├─ led_gpio_bright ─── gpiod_set_value ──── desc->gc->set() ─────→ vendor_a_set()
  │  ness_set(desc,1)    (desc, 1)           (..., 5, 1)          打印 DR_REG ← 0x20
  │                     │                    │                      │
  │  led_green (desc->gc = &vendorB) 走同样路径，落到 vendor_b_set()
```

## 文件职责与演进关系

| 文件 | 对应内核文件 | 职责 |
|------|------------|------|
| `gpio_chip.h` | `include/linux/gpio/driver.h` | 芯片抽象：gpio_chip 结构体 + consumer/provider API 声明 |
| `gpiolib.c` | `drivers/gpio/gpiolib.c` | 核心框架：chip 注册、查找、consumer API 实现 |
| `gpio_vendor_a.c` | `drivers/gpio/gpio-mxc.c` | 厂商 A 驱动：DR_REG 风格，片内 GPIO 控制器 |
| `gpio_vendor_b.c` | 类似 STM32 BSRR 实现 | 厂商 B 驱动：SET/CLR 风格，外扩 IO 扩展芯片 |
| `leds_gpio.c` | `drivers/leds/leds-gpio.c` | 设备驱动：调 gpiod_set_value，不关心底层芯片 |
| `main.c` | — | 模拟内核启动序列，串联所有层进行演示 |

演进关系：ch15 的 `platform_ops` 是"全局一份函数指针"，整个进程同一时刻只挂着一家芯片的实现。ch16 改为按 chip 实例分组，每个 chip 有自己的 ops 表，通过 `desc->gc` 反查 dispatch。这才贴近真实内核——同一颗 SoC 上可能并存多家厂商的 GPIO 控制器。

## 构建与运行

```bash
make        # 编译
make run    # 编译并运行
make clean  # 清理
```

## 本轮注释优化（2026-06-03）

本轮对所有源文件的注释进行了系统性增强，只操作注释，未改动一行代码逻辑：

- **`gpio_chip.h`** — 文件头部添加设计定位框、五层架构层级关系图；结构体字段逐字段 Doxygen 文档注释；每个 API 函数的用途/参数/返回值说明
- **`gpio_vendor_a.c`** — 硬件特征（DR_REG）说明框；第1层视角与第4层视角对照；每个函数的寄存器操作语义注解；chip 实例的"虚函数表填充"说明；第1层在五层架构中的位置
- **`gpio_vendor_b.c`** — BSRR 原子操作说明；与 vendor A 的差异对照；LOCK 寄存器设计意图；base/ngpio 的片内外扩用意；第1层在五层架构中的位置
- **`gpiolib.c`** — 框架层职责框；注册表管理逻辑；`desc->gc->set` dispatch 关键行带 ASCII 框的详细注解（含调用链全景）
- **`leds_gpio.c`** — 第4层驱动作者视角框；N×M→N+M ASCII 图解；函数 Doxygen 文档
- **`main.c`** — 3 阶段启动流程 ASCII 图；调用链全景跨文件数据流图；getchar 设计意图说明
- **`Makefile`** — 项目说明 + 每个构建目标的作用注释
