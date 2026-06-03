# Project Memory (Compact)

1) **ch16 Linux-style GPIO Subsystem** — 教学示例，来自《嵌入式 C 语言自我修养》第 16 章，路径 `oop-in-c/code/16-linux-style/pc/`。

2) **五层架构**：第5层 main.c（初始化/演示）→ 第4层 leds_gpio.c（设备驱动）→ 第3层 gpiolib.c（核心框架）→ 第2层 gpio_chip.h（芯片抽象接口）→ 第1层 vendor_a.c / vendor_b.c（硬件实现层）。每层只做一件事，层间通过接口对话。

3) **核心机制**：多态 dispatch。`gpiod_set_value` 内部一行 `desc->gc->set(...)`，根据 `desc->gc` 指向不同 chip 实例走到不同厂商的实现，相当于 C 语言虚函数表。

4) **gpiolib.c 是枢纽层** — 管理 chip 注册表、实现 consumer API、完成 dispatch 接线。理解它等于理解了整个设计的串联逻辑。

5) **N×M→N+M 解耦** — 中间层 gpiolib 将"设备数×芯片数"的代码量从乘法变为加法。芯片和设备越多，收益越显著。

6) **两家厂商实现**：vendor_a 用 DR_REG（数据寄存器，非原子读-改-写，模拟片内 GPIO 控制器，base=0, ngpio=32）；vendor_b 用 BSRR（SET/CLR 双寄存器，原子单 store，模拟外扩 IO 扩展芯片，base=32, ngpio=16）。寄存器命名差异（DIR vs MODE, DR vs BSRR），vendor_b 有 LOCK 寄存器解锁。

7) **base 和 ngpio 的 design intent** — `base` 是芯片在全局 GPIO 编号空间中的起始编号，`ngpio` 是引脚数，`desc->offset` 范围 0..ngpio-1。两者共同描述芯片在全局空间中的位置和容量。`base` 是内核 legacy 接口（已被 GPIO descriptor API 取代），保留是为了对齐 `include/linux/gpio/driver.h` 结构，本示例实际走 label 查找不走 base。

8) **struct gpio_chip = C 语言虚基类（父类）** — 5 个函数指针（request/free/direction_output/get/set）= 5 个虚方法。这不是 C++ 继承语法，而是"接口继承"——gpio_chip 定义契约（label + base/ngpio + 函数指针签名），vendor 实例化并填充实现（= 子类重写虚函数），通过 gpiochip_add() 注册。vendor 驱动填充这些指针后通过 gpiochip_add() 注册。

9) **struct gpio_desc** 是多态 dispatch 的钥匙 — (chip + offset) 对，避免每次调用遍历 chip 列表。

10) **演进关系**：ch15 用全局 platform_ops 指针（整个进程只有一套芯片实现）；ch16 改为按 chip 实例分组（多份 gpio_chip 共存，贴近真实 Linux 内核场景）。

11) **内核等价物**：gpio_chip.h → include/linux/gpio/driver.h；gpiolib.c → drivers/gpio/gpiolib.c；leds_gpio.c → drivers/leds/leds-gpio.c；vendor_a.c → drivers/gpio/gpio-mxc.c。

12) **项目文件清单**：gpio_chip.h, gpiolib.c, leds_gpio.c, main.c, gpio_vendor_a.c, gpio_vendor_b.c, Makefile, README.md。

13) **构建**：gcc -Wall -Wextra -std=c99，零 warning 零 error。`make` 编译，`make run` 编译+运行，`make clean` 清理。

14) **注释全面增强（2026-06-03，三轮）**：第一轮为所有源文件添加 Doxygen 字段文档、ASCII 架构框图、函数语义注释、调用链全景图；第二轮在 gpio_chip.h 中强化 base/ngpio 说明和"父类=接口继承"解释，在 gpiolib.c 中补充 base 历史作用说明；第三轮在 gpio_chip.h 中展开 N×M→N+M 要点（无抽象层 vs 有抽象层的增长对比），在 gpiolib.c 头部嵌入"有/无 gpiolib"代码对比框。只改注释，未动一行代码逻辑。

15) **README.md 已创建**（2026-06-03）— 收录五层架构图、核心设计思想（多态 dispatch / N×M→N+M / 每层只做一件事）、调用链全景、文件职责表。

16) **分层设计 rationale** — `led_gpio_brightness_set`（第4层，LED 语义）和 `gpiod_set_value`（第3层，GPIO dispatch）的分开设计体现了关注点分离：leds_gpio.c 作者不需要知道寄存器操作，gpiolib.c 作者不需要知道引脚接了 LED 还是马达。换芯片只改 vendor 层，不碰设备驱动。

17) **`.gitignore` 更新**（2026-06-03）— 添加 `.codewhale/`（IDE 配置，类似 .vscode/），构建产物 `*.gch`（GCC 预编译头）待加入忽略规则。
