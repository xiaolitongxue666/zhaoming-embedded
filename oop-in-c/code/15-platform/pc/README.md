# pc/ — PC 教学版四层架构

`make && ./demo` 直接在 PC 上跑 GPIO + PWM + I2C 三盏灯混搭流程。本章 **0 个新概念**，把 ch01–ch14 的 OOP 武器与 ch15 Platform 抽象组装成完整工程。

配套章节：[第 15 章 · Platform 抽象](../../../book/04-工程威力/15-Platform抽象.md)

## 编译运行

```bash
make
./demo
```

Windows 用户跑一次 `make` 即有 `demo.exe`。

---

## 四层架构（pc/ 教学视角）

层号 **从上往下**：离业务越近编号越小，离硬件越近编号越大。  
**层号 ≠ 启动顺序** —— Platform 虽是第 4 层（最底），但 `main()` 里必须 **最先** `platform_init()`。

| 层 | 文件 | 职责 | 换 PCB | 换 MCU |
|----|------|------|--------|--------|
| ① 应用 | `main.c` `app.c` `app.h` | 业务：自检 / 报警 / 状态指示 | 不改 | 不改 |
| ② 板级 BSP | `leds.h` `led_board_init.c` | 这块板的接线：ERR=GPIO pin10，NET=I2C 0x3C | **改这里** | 通常不改 |
| ③ 设备驱动 | `led_base.*` `led_gpio.*` `led_pwm.*` `led_i2c.*` | OOP 多态：父类 + 子类 + ops | 不改 | 不改 |
| ④ Platform | `platform_init.c` + `../platform/platform_*.c` + `platform_*_pc.c` + `../../common/platform_pc.c` | 硬件适配：dispatcher + 后端 | 不改 | **改后端** |

### 为什么 Board 是第 2 层

- 应用只 `#include "leds.h"`，拿 `struct led_base *` 句柄。
- 全工程 **唯一** 写 pin / PWM 通道 / I2C 地址的地方。
- 换板子（ERR 从 GPIO 改 PWM）只改 `led_board_init.c`，`app.c` 不动。
- Driver 是跨板复用的通用逻辑，不应写死 pin 号，所以 Board 在 Driver **上面**。

### 为什么 Platform 是第 4 层（最底）

- **依赖方向**：Driver 调 Platform，Platform 不调 Driver。
- **运行时链**：`led_on()` → `ops->on` → `platform_gpio_write()` / `platform_pwm_enable()` → 后端 → 硬件；**Board 不在此链中**（Board 只在开机 init 时出现）。
- **换 MCU** 只换 Platform 后端（PC 上为 `platform_*_pc.c`；真机上为 `platform/arch/<mcu>/`）。
- Platform 之下是厂家 HAL / 寄存器，不再建项目自己的层。

---

## 启动顺序 vs 运行时调用链

### 开机（只做一次）

```
main()
  platform_init()       ← L4：注册 PWM/I2C ops 到 dispatcher
  led_board_init()      ← L2：init 子类 + 绑 g_led_* 句柄
  power_on_test() …     ← L1：业务
```

### 开灯（每次 led_on 都走）

```
app.c: led_on(g_led_error)
  → led_base.c: me->ops->on
  → led_gpio.c: platform_gpio_write(pin, level)
  → common/platform_pc.c: printf
```

PWM / I2C 多经 dispatcher 转发（见下节）。

---

## Dispatcher（分发器）是什么

**不是**某个叫 `dispatcher` 的变量，而是 **设计模式**：中间层存一张 ops 函数表指针，对外固定 API，对内转发到启动时注册的后端。

与 ch11 `led_base + me->ops` 同构：

| | LED 驱动 | Platform PWM |
|--|----------|--------------|
| 上层调用 | `led_on(me)` | `platform_pwm_enable(ch)` |
| Dispatcher | `led_base.c` | `../platform/platform_pwm.c` |
| 具体实现 | `led_pwm.c` | `platform_pwm_pc.c` / `arch/stm32/pwm_board.c` |

### 代码位置

| 角色 | PWM | I2C |
|------|-----|-----|
| Dispatcher（存表 + 转发） | `../platform/platform_pwm.c` 的 `_g_ops` | `../platform/platform_i2c.c` 的 `_g_bus` |
| PC 后端（填 ops + register） | `platform_pwm_pc.c` | `platform_i2c_pc.c` |
| 注册入口 | `platform_init.c` → `platform_pc_*_init()` | 同左 |
| 上层使用者 | `led_pwm.c` | `led_i2c.c` / `led_board_init.c` |

### `platform_init()` 做什么

只做 **编排**，不写 LED、不碰寄存器：

```c
int platform_init(void)
{
    platform_pc_pwm_init();   /* platform_pwm_register(&pc_pwm_ops) */
    platform_pc_i2c_init();   /* platform_i2c_bus_register(...) */
    return 0;
}
```

跳过它 → `platform_i2c_bus_get()` 为 NULL，`platform_pwm_enable()` 因 `_g_ops == NULL` 崩溃。

### GPIO 为何不走 dispatcher

PC 版 GPIO 仍用 `../../common/platform_pc.c` 的 **直接函数**（`platform_gpio_write`），是 ch01–14 教学渐进保留。PWM / I2C 已升级为 ops + register；真机 PIN 见 `platform/platform_pin.c` + `arch/<mcu>/pin_board.c`。

---

## Platform 层的意义

1. **隔离厂家差异**：STM32 `HAL_GPIO_WritePin` vs NXP `GPIO_PinWrite` 收进 `arch/`，driver 字节不动。
2. **多外设共用**：PWM / I2C dispatcher 注册一次，LED、sensor、EEPROM 共用同一套 API。
3. **芯片隔离在 Platform 内部**：`platform/*.c`（dispatcher，跨 MCU 不变）+ `arch/<mcu>/` 或 `platform_*_pc.c`（后端，换 MCU 改这里）。下面直接接厂家 HAL，不再加第五层。

---

## 两套「四层」对照

| 视角 | 四层划分 | 用途 |
|------|----------|------|
| **pc/ 教学** | 应用 → 板级 → 驱动 → Platform | 强调 App / Board / Driver 封装纪律 |
| **换 MCU（上级 README）** | 应用 → drivers → platform 接口 → arch 实现 | 强调移植时哪几份文件要改 |

Board 在换 MCU 视角下通常与 App 一样 **不改**；在 pc 教学视角下 **单独成层**，因为换 PCB 接线只动 `led_board_init.c`。

---

## 文件与 Makefile 对应

```
pc/
  main.c app.c app.h              ① 应用
  leds.h led_board_init.c         ② 板级
  led_base.* led_gpio.* …         ③ 驱动
  platform_init.c                 ④ 注册入口
  platform_pwm_pc.c platform_i2c_pc.c   ④ PC 后端
../platform/platform_pwm.c platform_i2c.c  ④ dispatcher
../../common/platform_pc.c        ④ GPIO 简版后端
```

`CFLAGS -I..` 使 `#include "platform/platform_pwm.h"` 与真机工程路径一致，driver 源码可字节级复用。
