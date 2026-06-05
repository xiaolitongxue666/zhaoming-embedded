# 学习计划 · C 语言面向对象编程·嵌入式实战

在线书：[zhaochengbo.github.io/zhaoming-embedded](https://zhaochengbo.github.io/zhaoming-embedded/)

本文件与 `practice/` 目录同步更新。建议每章完成后在「总进度」中打勾，并在「完成日期」列填写实际日期。下文 **practice ↔ oop-in-c** 对照以 `practice/` 路径为索引，补充 [`book/附录/D-配套代码索引.md`](../book/附录/D-配套代码索引.md) 中的官方目录视角。

---

## 学习原则

1. **先读再做**：读完对应在线章节后再打开 `practice/.../pc/`。
2. **先关参考**：手写阶段不要打开 `oop-in-c/code/` 同名目录。
3. **编译验收**：`gcc -Wall -Wextra` 0 警告，`./demo` 输出符合本章 README。
4. **再对比**：通过验收后再对照官方教学包，记录差异与疑问。

---

## practice 与 oop-in-c 对照总则

### 目录命名差异

| 位置 | 路径格式 | 示例 |
|------|----------|------|
| **practice**（手写区） | `阶段名/chNN-主题/pc/` | `01-封装/ch01-three-leds/pc/` |
| **oop-in-c**（官方参考） | `code/NN-主题/`（无 `ch` 前缀） | `code/01-three-leds/` |

章号与主题 slug 与在线书一致；找对照代码时去掉 `ch`、改用 `NN-` 即可。

### 结构差异

| | practice | oop-in-c |
|---|----------|----------|
| **pc/** | 骨架：`main.c`、`Makefile`、`README.md`；业务 `.c/.h` 由你手写 | 完整可编译参考；`Makefile` 已列全源文件 |
| **平台层** | 各章 `Makefile` 链接 [`oop-in-c/code/common/`](../oop-in-c/code/common/) 的 `platform_pc.c` | 同上；ch01–ch14 多数另有 **`platform-mcu/stm32/`** 真机片段 |
| **扩展** | 无 | ch15 起含 `drivers/`、`platform/`、`linux-driver/` 等（见分阶段说明） |

### 对照三条规则

| 规则 | 说明 |
|------|------|
| **手写区** | 只改 `practice/**/pc/` 下业务 `.c/.h`；GPIO 模拟用共用 `common/platform_pc.c`，不必每章重写 |
| **对照区** | 验收通过后再打开 `oop-in-c/code/<NN-*/>` |
| **优先子目录** | 与练习同名的 **`pc/`** 先对照；真机 / 内核 / 跨 MCU 扩展见各章 `platform-mcu/`、`linux-driver/`、`drivers/`、`platform/`（验收后再看） |

### 共用平台层（ch01–ch18）

- [`oop-in-c/code/common/platform.h`](../oop-in-c/code/common/platform.h) — `platform_gpio_init` / `platform_gpio_write` 等接口
- [`oop-in-c/code/common/platform_pc.c`](../oop-in-c/code/common/platform_pc.c) — PC 上 printf 模拟 GPIO

practice 各章 `Makefile` 中 `COMMON = ../../../../oop-in-c/code/common`，与官方 `pc/Makefile` 一致。

### 阶段总对照表

| 阶段 | practice 路径 | oop-in-c 根目录（相对 `oop-in-c/code/`） | 主要对照子目录 | 备注 |
|------|---------------|------------------------------------------|----------------|------|
| 0 序曲 | [`00-序曲/`](00-序曲/)（无 `pc/`） | [`01-three-leds/`](../oop-in-c/code/01-three-leds/) | **`pc/`** 预热 | 序曲不写代码，只跑官方 ch01 demo |
| 1 封装 | `01-封装/ch01`…`ch05` | `01-three-leds` … `05-hal-mapping` | **`pc/`** + 多数章 `platform-mcu/stm32/` | ch05 无 LED 类，为 HAL 分层 demo |
| 2 继承 | [`02-继承/ch06-inherit-pain/`](02-继承/ch06-inherit-pain/) | [`06-inherit-pain/`](../oop-in-c/code/06-inherit-pain/) | **`pc/`** | 父类 + gpio/pwm 子类 |
| 3 多态 | `03-多态/ch07`…`ch11` | `07-function-pointer` … `11-polymorphism` | **`pc/`**；ch09–11 的 stm32 按子类拆分 | ch07 官方仅 `main.c` |
| 4 工程威力 | `04-工程威力/ch12`…`ch18` | `12-upcasting` … `18-roadmap` | **`pc/`**；ch15–17 有多棵扩展子树 | ch18 官方仅 `main.c` 回顾 |
| 5 工业实战 | [`05-工业实战/`](05-工业实战/)（无 `pc/`） | — | [`industrial-zephyr/`](../industrial-zephyr/)、[`industrial-linux/`](../industrial-linux/) | 概念可回溯 oop-in-c ch11–ch17 |

---

## 环境准备

| 步骤 | 说明 |
|------|------|
| 安装 GCC + make | 见 [`setup/README.md`](../setup/README.md) |
| 克隆本仓库 | 路径尽量使用纯英文，避免中文路径编译问题 |
| 验证（practice 骨架） | `cd practice/01-封装/ch01-three-leds/pc && make && ./demo`（仅 `main` + `platform_pc`，手写 `led.*` 后需在 `Makefile` 的 `SRCS` 追加） |
| 验证（官方完整包） | `cd oop-in-c/code/01-three-leds/pc && make && ./demo` |

---

## 总进度

在 `[ ]` 中改为 `[x]` 表示完成。

| 阶段 | 章节 | 进度 | 完成日期 |
|------|------|------|----------|
| 0 序曲 | 序曲 | [ ] | |
| 1 封装 | ch01 | [x] | |
| 1 封装 | ch02 | [ ] | |
| 1 封装 | ch03 | [ ] | |
| 1 封装 | ch04 | [ ] | |
| 1 封装 | ch05 | [ ] | |
| 2 继承 | ch06 | [ ] | |
| 3 多态 | ch07 | [ ] | |
| 3 多态 | ch08 | [ ] | |
| 3 多态 | ch09 | [ ] | |
| 3 多态 | ch10 | [ ] | |
| 3 多态 | ch11 | [ ] | |
| 4 工程威力 | ch12 | [ ] | |
| 4 工程威力 | ch13 | [ ] | |
| 4 工程威力 | ch14 | [ ] | |
| 4 工程威力 | ch15 | [ ] | |
| 4 工程威力 | ch16 | [ ] | |
| 4 工程威力 | ch17 | [ ] | |
| 4 工程威力 | ch18 | [ ] | |
| 5 工业实战 | ch19 | [ ] | |
| 5 工业实战 | ch20 | [ ] | |
| 附录 | 附录 B（Zephyr） | [ ] 可选 | |
| 附录 | 附录 C（Linux） | [ ] 可选 | |
| 附录 | 附录 D（代码索引） | [ ] | |

---

## 分阶段路径

### 阶段 0 · 序曲（约 0.5 天）

- 阅读：[5 分钟看见你的第一个 OOP LED](https://zhaochengbo.github.io/zhaoming-embedded/00-序曲/00-五分钟看见OOP.html)
- **practice**：[`00-序曲/README.md`](00-序曲/README.md) — 无 `pc/`，不写代码
- **oop-in-c 预热**：[`01-three-leds/pc/`](../oop-in-c/code/01-three-leds/pc/) — `led.h`、`led.c`、`main.c` + `common/platform_pc.c`
- **可选延伸**（不必深究）：[`01-three-leds/platform-mcu/stm32/led_stm32.c`](../oop-in-c/code/01-three-leds/platform-mcu/stm32/led_stm32.c) — PIN 编码与 ch15 一致

```bash
cd oop-in-c/code/01-three-leds/pc && make && ./demo
```

### 阶段 1 · 封装 ch01–ch05（建议每章 3–7 天）

**ch01–ch10 共性**：官方章根 `README.md` + `pc/README.md` 描述 PC / STM32 双轨；practice 只做 **PC 轨**（`pc/`）。

| 章 | 练习目录 | 官方参考（oop-in-c） | 在线阅读 |
|----|----------|----------------------|----------|
| ch01 | [`01-封装/ch01-three-leds/pc/`](01-封装/ch01-three-leds/pc/) | [`01-three-leds/`](../oop-in-c/code/01-three-leds/) | [三个 LED 三份代码](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/01-三个LED三份代码.html) |
| ch02 | [`01-封装/ch02-static-hiding/pc/`](01-封装/ch02-static-hiding/pc/) | [`02-static-hiding/`](../oop-in-c/code/02-static-hiding/) | [同事改了一行 · static](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/02-同事改了一行.html) |
| ch03 | [`01-封装/ch03-handwritten-class/pc/`](01-封装/ch03-handwritten-class/pc/) | [`03-handwritten-class/`](../oop-in-c/code/03-handwritten-class/) | [手搓 class](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/03-手搓class.html) |
| ch04 | [`01-封装/ch04-data-classification/pc/`](01-封装/ch04-data-classification/pc/) | [`04-data-classification/`](../oop-in-c/code/04-data-classification/) | [数据三级分类](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/04-数据归位.html) |
| ch05 | [`01-封装/ch05-hal-mapping/pc/`](01-封装/ch05-hal-mapping/pc/) | [`05-hal-mapping/`](../oop-in-c/code/05-hal-mapping/) | [HAL 映射](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/05-HAL映射.html) |

| 章 | practice `pc/` 手写重点 | 官方 `pc/` 参考文件 | 其他子目录（验收后） |
|----|-------------------------|---------------------|----------------------|
| ch01 | `led.h`, `led.c` | `led.h/c`, `main.c` | `platform-mcu/stm32/led_stm32.c` |
| ch02 | ch01 + `static`、`led_get_state` | 同上结构 | `platform-mcu/stm32/led_stm32.c` |
| ch03 | `led.*` + `motor.*` | `led.*`, `motor.*`, `main.c` | `platform-mcu/stm32/led_motor_stm32.c` |
| ch04 | 数据归位 | `led.*`, `main.c` | **`pc/led_bad.c/h`**（反面教材）、`platform-mcu/stm32/` |
| ch05 | 偏阅读；可选最小 HAL | `gpio_typedef.h`, `hal_gpio.h/c`, `main.c`（无 LED 类） | `platform-mcu/stm32/hal_gpio_real.c` |

### 阶段 2 · 继承 ch06（建议 5–7 天）

| 章 | 练习目录 | 官方参考（oop-in-c） | 在线阅读 |
|----|----------|----------------------|----------|
| ch06 | [`02-继承/ch06-inherit-pain/pc/`](02-继承/ch06-inherit-pain/pc/) | [`06-inherit-pain/`](../oop-in-c/code/06-inherit-pain/) | [代码一半重复](https://zhaochengbo.github.io/zhaoming-embedded/02-继承/06-代码一半重复.html) |

- **手写**：`led_base.*`、`led_gpio.*`、`led_pwm.*`
- **官方 `pc/`**：上述 6 个模块 + `main.c`
- **`platform-mcu/stm32/`**：`led_stm32.c`（套入 CubeMX 的片段，见章根 README）

### 阶段 3 · 多态 ch07–ch11（建议每章 4–7 天）

| 章 | 练习目录 | 官方参考（oop-in-c） | 在线阅读 |
|----|----------|----------------------|----------|
| ch07 | [`03-多态/ch07-function-pointer/pc/`](03-多态/ch07-function-pointer/pc/) | [`07-function-pointer/`](../oop-in-c/code/07-function-pointer/) | [写死的函数怎么换](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/07-写死的函数怎么换.html) |
| ch08 | [`03-多态/ch08-callback/pc/`](03-多态/ch08-callback/pc/) | [`08-callback/`](../oop-in-c/code/08-callback/) | [把号码给别人拨](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/08-把号码给别人拨.html) |
| ch09 | [`03-多态/ch09-ops-table/pc/`](03-多态/ch09-ops-table/pc/) | [`09-ops-table/`](../oop-in-c/code/09-ops-table/) | [ops 操作表](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/09-ops操作表.html) |
| ch10 | [`03-多态/ch10-vptr/pc/`](03-多态/ch10-vptr/pc/) | [`10-vptr/`](../oop-in-c/code/10-vptr/) | [ops 放进对象](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/10-ops放进对象.html) |
| ch11 | [`03-多态/ch11-polymorphism/pc/`](03-多态/ch11-polymorphism/pc/) | [`11-polymorphism/`](../oop-in-c/code/11-polymorphism/) | [多态完整图景](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/11-多态完整图景.html) |

| 章 | 官方 `pc/` 要点 | 演进说明 |
|----|-----------------|----------|
| ch07 | **仅 `main.c`**（`gpio_on` / `pwm_on` / `i2c_on` + 函数指针） | 无 LED 类 |
| ch08 | `led.h/c`, `main.c` | 回调注册 |
| ch09 | `led_base.*`, `led_gpio.*`, `led_pwm.*` | ops 表外置 |
| ch10 | 同 ch09 | `led_base` 内嵌 `const struct led_ops *ops` |
| ch11 | 增加 **`led_i2c.*`** | 三子类 + 统一 `led_on` |

**ch09–ch11**：`platform-mcu/stm32/` 下为 `led_gpio.c`、`led_pwm.c`、`led_i2c.c`（与 `pc/` 子类逻辑对应）。

### 阶段 4 · 工程威力 ch12–ch18（建议每章 5–10 天）

| 章 | 练习目录 | 官方参考（oop-in-c） | 在线阅读 |
|----|----------|----------------------|----------|
| ch12 | [`04-工程威力/ch12-upcasting/pc/`](04-工程威力/ch12-upcasting/pc/) | [`12-upcasting/`](../oop-in-c/code/12-upcasting/) | [向上转型](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/12-向上转型.html) |
| ch13 | [`04-工程威力/ch13-container-of/pc/`](04-工程威力/ch13-container-of/pc/) | [`13-container-of/`](../oop-in-c/code/13-container-of/) | [container_of](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/13-container_of.html) |
| ch14 | [`04-工程威力/ch14-pure-virtual/pc/`](04-工程威力/ch14-pure-virtual/pc/) | [`14-pure-virtual/`](../oop-in-c/code/14-pure-virtual/) | [纯虚与抽象类](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/14-纯虚与抽象类.html) |
| ch15 | [`04-工程威力/ch15-platform/pc/`](04-工程威力/ch15-platform/pc/) | [`15-platform/`](../oop-in-c/code/15-platform/) | [Platform 抽象](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/15-Platform抽象.html) |
| ch16 | [`04-工程威力/ch16-linux-style/pc/`](04-工程威力/ch16-linux-style/pc/) | [`16-linux-style/`](../oop-in-c/code/16-linux-style/) | [Linux 不难](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/16-Linux不难.html) |
| ch17 | [`04-工程威力/ch17-initcall/pc/`](04-工程威力/ch17-initcall/pc/) | [`17-initcall/`](../oop-in-c/code/17-initcall/) | [initcall](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/17-initcall.html) |
| ch18 | [`04-工程威力/ch18-roadmap/pc/`](04-工程威力/ch18-roadmap/pc/) | [`18-roadmap/`](../oop-in-c/code/18-roadmap/) | [全书地图](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/18-全书地图.html) |

| 章 | 主要对照 `pc/` | 扩展子目录（验收后阅读） |
|----|----------------|---------------------------|
| ch12 | `led_base.*`，三子类，**`led_board_init.c`**，**`leds.h`**，`main.c` | `platform-mcu/stm32/`（含 `led_board_init.c`） |
| ch13 | ch12 文件集 + **`container_of.h`** | stm32 子目录同结构 |
| ch14 | LED 栈 + **`sensor_*`**（`sensor_base/temp`、`sensor_board_init`、`sensors.h`） | `platform-mcu/stm32/led_gpio.c`、`led_pwm.c` |
| ch15 | **`app.c/h`**，`platform_init.*`，`platform_*_pc.c`，全套 `led_*`，`container_of.h` | **`drivers/led/`**（跨 MCU）、**`platform/`** + **`platform/arch/stm32|nxp/`**、**`linux-driver/userspace/`** |
| ch16 | **`gpiolib.c`**，`gpio_chip.h`，`gpio_vendor_a/b.c`，`leds_gpio.c`，`main.c` | `linux-driver/userspace/led_userspace.c`；`platform-mcu/stm32/README.md` 为说明 |
| ch17 | **`initcall.h/c`**，**`drv_led/uart/i2c/spi.c`**，`main.c` | `platform-mcu/stm32/README.md`（链接脚本）；`linux-driver/README.md`（内核 initcall 索引） |
| ch18 | **`main.c` 单文件** 五阶段回顾 | 章根 `README.md` |

**ch15 特别说明**：practice 的 `pc/` 对应官方 **`15-platform/pc/`**（四层教学版，`make && ./demo`）。工业完整分层在 **`drivers/led/`** + **`platform/`** + **`platform/arch/<mcu>/`** — 换 MCU 只改 `arch` 下 6 份 board 文件，应用与驱动层一字不动（书 §15.11）。

### 阶段 5 · 工业实战 + 附录（硬件可选）

- **practice**：[`05-工业实战/README.md`](05-工业实战/README.md) — 无 `pc/`
- **ch19**：[在线阅读](https://zhaochengbo.github.io/zhaoming-embedded/05-工业实战/19-主控案例.html) · [`industrial-zephyr/`](../industrial-zephyr/)（`src/main_demo1_4led.c` … `main_demo4_enosys.c`）
- **ch20**：[在线阅读](https://zhaochengbo.github.io/zhaoming-embedded/05-工业实战/20-子控案例.html) · [`industrial-linux/`](../industrial-linux/)（`ch20-leds-status/`、`ch20-demo2-libgpiod/` 等）

**oop-in-c 概念回溯**（读 Zephyr / Linux 工程时按需打开）：

| 书章主题 | 建议回溯 oop-in-c |
|----------|-------------------|
| Zephyr driver / ops / ENOSYS | `11-polymorphism`、`14-pure-virtual` |
| overlay / 板级配置 | `12-upcasting`、`15-platform` |
| CONTAINER_OF 回调 | `13-container-of` |
| module_init / 自动注册 | `17-initcall` |

---

## 逐章任务卡片

对比顺序统一为：**practice 验收** → `cd oop-in-c/code/<章>/pc && make && ./demo` → 再读扩展子目录。

### ch01 · 三个 LED 三份代码

- **概念**：重复代码痛点；`struct led` + `me` 指针；`platform_*` 胶水层
- **手写**：`led.h`、`led.c`（三实例共用一套 API）
- **验收**：三颗 LED（Pin 13/14/15）依次点亮，GPIO 日志来自 `platform_pc.c`
- **官方参考结构（oop-in-c）**
  - 根目录：[`oop-in-c/code/01-three-leds/`](../oop-in-c/code/01-three-leds/)
  - **`pc/`**（与 practice 同构）：`led.h`、`led.c`、`main.c`、`Makefile` → `common/platform_pc.c`
  - **`platform-mcu/stm32/`**：`led_stm32.c` — PIN_NUM 编码；应用层 `led.c` 不改
  - 说明：章根 `README.md`、`pc/README.md`

### ch02 · static 与信息隐藏

- **概念**：`static` 文件内可见；`/* private */` 约定；读状态走 API
- **手写**：在 ch01 基础上加 `led_get_state`、内部 `static` 工具函数
- **验收**：外部无法误改内部状态（逻辑上通过 API 访问）
- **官方参考结构（oop-in-c）**
  - 根目录：[`02-static-hiding/`](../oop-in-c/code/02-static-hiding/)
  - **`pc/`**：`led.h/c`（含 static 与 getter）、`main.c`
  - **`platform-mcu/stm32/`**：`led_stm32.c`

### ch03 · 手搓 class

- **概念**：`模块_动作` 命名；句柄 + 操作函数；LED 与 motor 同套路
- **手写**：`led.h/c`、`motor.h/c`；`main.c` 演示两模块并列
- **验收**：`led_init` / `motor_init` 无符号冲突，行为符合书内示例
- **官方参考结构（oop-in-c）**
  - 根目录：[`03-handwritten-class/`](../oop-in-c/code/03-handwritten-class/)
  - **`pc/`**：`led.*`、`motor.*`、`main.c`
  - **`platform-mcu/stm32/`**：`led_motor_stm32.c`

### ch04 · 数据三级分类

- **概念**：实例 / 模块 / 全局数据归位；反面 `led_bad` 对比（可选）
- **手写**：按书重构数据归属；理清哪些字段进 `struct`
- **验收**：无多余全局可变状态驱动 LED 行为
- **官方参考结构（oop-in-c）**
  - 根目录：[`04-data-classification/`](../oop-in-c/code/04-data-classification/)
  - **`pc/`**：`led.*`、`main.c`
  - **验收后对照**：`pc/led_bad.c`、`led_bad.h`（错误归位示范）
  - **`platform-mcu/stm32/`**：`led_stm32.c`

### ch05 · HAL 映射（偏阅读）

- **概念**：抽象接口 → 平台实现；对照 ST HAL 头文件分层
- **手写**：以阅读 + 笔记为主；可选在 `pc/` 写最小 platform 对照表
- **验收**：能画出应用 / 驱动 / HAL / 寄存器四层关系
- **官方参考结构（oop-in-c）**
  - 根目录：[`05-hal-mapping/`](../oop-in-c/code/05-hal-mapping/)
  - **`pc/`**：`gpio_typedef.h`、`hal_gpio.h/c`、`main.c`（演示 HAL 分层，无 `struct led`）
  - **`platform-mcu/stm32/`**：`hal_gpio_real.c`

### ch06 · 继承痛点

- **概念**：`struct led_base` + 子类 `led_gpio` / `led_pwm`；子类 init 先调父类
- **手写**：`led_base.*`、`led_gpio.*`、`led_pwm.*`
- **验收**：子类复用父类字段布局，减少重复代码
- **官方参考结构（oop-in-c）**
  - 根目录：[`06-inherit-pain/`](../oop-in-c/code/06-inherit-pain/)
  - **`pc/`**：`led_base.*`、`led_gpio.*`、`led_pwm.*`、`main.c`
  - **`platform-mcu/stm32/`**：`led_stm32.c`

### ch07 · 函数指针入门

- **概念**：函数名 = 地址；`void (*fp)(int)` 换实现
- **手写**：`main.c` 中 `gpio_on` / `pwm_on` / `i2c_on` + 函数指针切换
- **验收**：切换 `fp` 后打印不同实现路径
- **官方参考结构（oop-in-c）**
  - 根目录：[`07-function-pointer/`](../oop-in-c/code/07-function-pointer/)
  - **`pc/`**：**仅 `main.c`** + `Makefile`（无 `led.h`）
  - **`platform-mcu/stm32/`**：`led_stm32.c`（与 ch07 主题弱相关，可略）

### ch08 · callback

- **概念**：把函数指针当参数传入；注册回调
- **手写**：`led.c` / `led.h` 带 callback 字段或注册接口
- **验收**：main 注册不同底层实现，LED API 行为一致
- **官方参考结构（oop-in-c）**
  - 根目录：[`08-callback/`](../oop-in-c/code/08-callback/)
  - **`pc/`**：`led.h/c`、`main.c`
  - **`platform-mcu/stm32/`**：`led_stm32.c`

### ch09 · ops 操作表

- **概念**：`struct led_ops` 表；`led_ops_gpio` / `led_ops_pwm` 实例
- **手写**：ops 表定义与填充，通过表调用
- **验收**：换表即换实现，main 不碰具体驱动函数名
- **官方参考结构（oop-in-c）**
  - 根目录：[`09-ops-table/`](../oop-in-c/code/09-ops-table/)
  - **`pc/`**：`led_base.*`、`led_gpio.*`、`led_pwm.*`、`main.c`
  - **`platform-mcu/stm32/`**：`led_gpio.c`、`led_pwm.c`

### ch10 · vptr 落地

- **概念**：ops 指针放进 `struct led_base`；dispatch 经 `ops->on`
- **手写**：base 含 `const struct led_ops *ops`
- **验收**：`led_on(led)` 内部走 `led->ops->on(led)`
- **官方参考结构（oop-in-c）**
  - 根目录：[`10-vptr/`](../oop-in-c/code/10-vptr/)
  - **`pc/`**：同 ch09 文件名；差异在 `led_base` 内嵌 ops 指针
  - **`platform-mcu/stm32/`**：`led_gpio.c`、`led_pwm.c`

### ch11 · 多态完整图景

- **概念**：gpio / pwm / i2c 子类 + 统一 `led_on` 胶水
- **手写**：完整子类集 + 对外 API
- **验收**：同一句 `led_on` 驱动不同子类实例
- **官方参考结构（oop-in-c）**
  - 根目录：[`11-polymorphism/`](../oop-in-c/code/11-polymorphism/)
  - **`pc/`**：ch10 文件集 + **`led_i2c.h/c`**
  - **`platform-mcu/stm32/`**：`led_gpio.c`、`led_pwm.c`、`led_i2c.c`

### ch12 · 向上转型

- **概念**：`struct led_base *` 指向任意子类；数组统一遍历
- **手写**：基类指针数组 + 循环 `led_on`
- **验收**：一个循环点亮 gpio / pwm / i2c 实例
- **官方参考结构（oop-in-c）**
  - 根目录：[`12-upcasting/`](../oop-in-c/code/12-upcasting/)
  - **`pc/`**：三子类 + `led_base.*`、**`led_board_init.c`**、**`leds.h`**、`main.c`
  - **`platform-mcu/stm32/`**：同上 + `led_board_init.c`

### ch13 · container_of

- **概念**：从成员指针反推容器；`offsetof` 公式
- **手写**：实现或使用 `container_of` 宏，从 `base` 找回子类
- **验收**：回调只收到 `base *` 时能调用子类特有逻辑
- **官方参考结构（oop-in-c）**
  - 根目录：[`13-container-of/`](../oop-in-c/code/13-container-of/)
  - **`pc/`**：ch12 文件集 + **`container_of.h`**
  - **`platform-mcu/stm32/`**：`led_gpio.c`、`led_i2c.c`、`led_pwm.c`（无 board_init 在部分章为拆分驱动）

### ch14 · 纯虚与抽象类

- **概念**：ops 某成员为 NULL / `-ENOSYS`；板级 init 表
- **手写**：抽象 ops + `sensor_board_init` 一类板级装配
- **验收**：未实现的 op 有明确失败路径
- **官方参考结构（oop-in-c）**
  - 根目录：[`14-pure-virtual/`](../oop-in-c/code/14-pure-virtual/)
  - **`pc/`**：LED 栈（同 ch13）+ **`sensor_base.*`**、**`sensor_temp.*`**、**`sensor_board_init.c`**、**`sensors.h`**
  - **`platform-mcu/stm32/`**：`led_gpio.c`、`led_pwm.c`

### ch15 · Platform 抽象

- **概念**：四层架构；换平台只改 platform / 驱动
- **手写**：platform 初始化 + 驱动注册；应用层稳定
- **验收**：PC 与书中分层一致，应用 `.c` 不直接调 `printf` GPIO
- **官方参考结构（oop-in-c）**
  - 根目录：[`15-platform/`](../oop-in-c/code/15-platform/)
  - **先对照 `pc/`**：`app.c/h`、`main.c`、`platform_init.*`、`platform_pin_pwm_i2c_*_pc.c`、`led_*`、`led_board_init.c`、`container_of.h`
  - **验收后 `drivers/led/`**：`led_base/gpio/pwm/i2c` — 跨 MCU 不变
  - **验收后 `platform/`**：`platform_pin/pwm/i2c.*` + **`platform/arch/stm32/`**、**`arch/nxp/`** 各 3 份 `*_board.c`
  - **可选 `linux-driver/userspace/`**：用户态 libgpiod 风格对照

### ch16 · Linux 风格

- **概念**：tab 缩进；`struct gpio_chip`；gpiolib 风格封装
- **手写**：按书整理为 Linux 内核命名与分层习惯
- **验收**：代码风格与官方 ch16 demo 同级
- **官方参考结构（oop-in-c）**
  - 根目录：[`16-linux-style/`](../oop-in-c/code/16-linux-style/)
  - **`pc/`**：`gpiolib.c`、`gpio_chip.h`、`gpio_vendor_a.c`、`gpio_vendor_b.c`、`leds_gpio.c`、`main.c`
  - **`linux-driver/userspace/`**：`led_userspace.c`、`Makefile`

### ch17 · initcall

- **概念**：链接段收集 init；`MODULE_INIT` 宏
- **手写**：`initcall.h` + 多个 `drv_*.c` 自动注册
- **验收**：main 不显式调用各驱动 init，遍历段表即可
- **官方参考结构（oop-in-c）**
  - 根目录：[`17-initcall/`](../oop-in-c/code/17-initcall/)
  - **`pc/`**：`initcall.h/c`、`drv_led.c`、`drv_uart.c`、`drv_i2c.c`、`drv_spi.c`、`main.c`
  - **`platform-mcu/stm32/README.md`**：链接脚本片段说明
  - **`linux-driver/README.md`**：内核 `__init` / initcall 源码路径索引

### ch18 · 全书地图

- **概念**：一颗 LED 的演化五阶段回顾
- **手写**：`main.c` 串联打印或调用各阶段代表性 API
- **验收**：运行 demo 能复述从 ch01 到 ch17 的演进
- **官方参考结构（oop-in-c）**
  - 根目录：[`18-roadmap/`](../oop-in-c/code/18-roadmap/)
  - **`pc/`**：**仅 `main.c`** + `Makefile`（打印/调用各阶段代表 API）
  - 章根 **`README.md`**：五阶段叙事，无额外子目录

### ch19 · Zephyr 实战（无 practice/pc）

- **阅读**：[主控案例](https://zhaochengbo.github.io/zhaoming-embedded/05-工业实战/19-主控案例.html)
- **动手**：[`industrial-zephyr/`](../industrial-zephyr/) — `west build -b stm32f4_disco -p auto -- -DDEMO=1`（DEMO=1…4）
- **oop-in-c 回溯**：`11-polymorphism`、`13-container-of`、`14-pure-virtual`、`15-platform`
- **附录**：[附录 B](https://zhaochengbo.github.io/zhaoming-embedded/附录/B-STM32完整工程.html)

### ch20 · Linux 实战（无 practice/pc）

- **阅读**：[子控案例](https://zhaochengbo.github.io/zhaoming-embedded/05-工业实战/20-子控案例.html)
- **动手**：[`industrial-linux/ch20-leds-status/`](../industrial-linux/ch20-leds-status/) 等
- **oop-in-c 回溯**：`13-container-of`、`16-linux-style`、`17-initcall`
- **附录**：[附录 C](https://zhaochengbo.github.io/zhaoming-embedded/附录/C-Linux完整工程.html)

---

## practice 目录树 · oop-in-c 完整索引

便于 Ctrl+F：左列为 **practice** 相对路径（以本文件所在 `practice/` 为根），中为 **oop-in-c**（相对 `oop-in-c/code/`），右列为本章 demo 要证明的一点。

```
practice/
├── 00-序曲/
│   └── （无 pc）                    → 01-three-leds/pc          预热：struct+me+platform 第一印象
├── 01-封装/
│   ├── ch01-three-leds/pc/          → 01-three-leds/pc          三实例共用 led_on，消灭重复代码
│   │                                → …/platform-mcu/stm32/led_stm32.c
│   ├── ch02-static-hiding/pc/       → 02-static-hiding/pc       static 隐藏实现细节
│   ├── ch03-handwritten-class/pc/   → 03-handwritten-class/pc   led+motor 双「类」并列
│   ├── ch04-data-classification/pc/ → 04-data-classification/pc 数据归位；pc/led_bad 反面
│   └── ch05-hal-mapping/pc/         → 05-hal-mapping/pc         HAL 四层，无 OOP LED
├── 02-继承/
│   └── ch06-inherit-pain/pc/        → 06-inherit-pain/pc        嵌套 struct = 继承
├── 03-多态/
│   ├── ch07-function-pointer/pc/    → 07-function-pointer/pc    仅 main：函数指针换实现
│   ├── ch08-callback/pc/            → 08-callback/pc            回调注入底层
│   ├── ch09-ops-table/pc/           → 09-ops-table/pc           外置 ops 表
│   ├── ch10-vptr/pc/                → 10-vptr/pc                ops 指针进对象
│   └── ch11-polymorphism/pc/        → 11-polymorphism/pc        gpio+pwm+i2c 统一 led_on
├── 04-工程威力/
│   ├── ch12-upcasting/pc/           → 12-upcasting/pc           基类指针数组遍历
│   ├── ch13-container-of/pc/        → 13-container-of/pc        从 base 成员找回子类
│   ├── ch14-pure-virtual/pc/        → 14-pure-virtual/pc        可空 ops + sensor 第二套
│   ├── ch15-platform/pc/            → 15-platform/pc            四层 PC 教学版
│   │                                → 15-platform/drivers/led/  跨 MCU 驱动层
│   │                                → 15-platform/platform/     接口 + arch/stm32|nxp
│   │                                → 15-platform/linux-driver/userspace/
│   ├── ch16-linux-style/pc/         → 16-linux-style/pc         gpiolib + gpio_chip
│   ├── ch17-initcall/pc/            → 17-initcall/pc            MODULE_INIT 段遍历
│   └── ch18-roadmap/pc/             → 18-roadmap/pc             ch01–ch17 演化回顾
└── 05-工业实战/
    └── （无 pc）                    → industrial-zephyr/        ch19 Zephyr 四 demo
                                     → industrial-linux/       ch20 内核驱动等
                                     → oop-in-c 回溯见阶段 5 表
```

**共用**：所有 `practice/**/pc/Makefile` → `oop-in-c/code/common/platform.{h,c}`。

---

## 无硬件时的备选路径

与书附录 D.4 一致：

1. ch01–ch18：只做 `practice/**/pc` + 对照 `oop-in-c/code/<NN-*/pc`（扩展子目录仅阅读）
2. ch19：读 `industrial-zephyr/src` + Zephyr 上游 driver 源码；概念回溯 `11`/`13`/`14`/`15`
3. ch20：读 `industrial-linux` + 书内 `leds-status.c` 片段；概念回溯 `13`/`16`/`17`

---

## 配套资源

| 资源 | 链接 |
|------|------|
| 在线书 | https://zhaochengbo.github.io/zhaoming-embedded/ |
| 官方教学包 | [`oop-in-c/code/`](../oop-in-c/code/) |
| 代码索引（书附录 D） | [`book/附录/D-配套代码索引.md`](../book/附录/D-配套代码索引.md) — 与本文「阶段总对照表」「目录树索引」互补：D 按章列目录，本文按 **practice 路径** 列手写区与对照区 |
| 环境配置 | [`setup/README.md`](../setup/README.md) |
| B 站 | 搜「兆鸣嵌入式」 |
