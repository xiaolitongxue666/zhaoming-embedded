# Project Memory (Compact)

1) **ch16 Linux-style GPIO Subsystem** — 教学示例，来自《嵌入式 C 语言自我修养》第 16 章，路径 `oop-in-c/code/16-linux-style/pc/`。

2) **五层架构**：第5层 main.c（初始化/演示）→ 第4层 leds_gpio.c（设备驱动）→ 第3层 gpiolib.c（核心框架）→ 第2层 gpio_chip.h（芯片抽象接口）→ 第1层 vendor_a.c / vendor_b.c（硬件实现层）。

3) **核心机制**：多态 dispatch。`gpiod_set_value` 内部一行 `desc->gc->set(...)`，根据 `desc->gc` 指向不同 chip 实例走到不同厂商的实现，相当于 C 语言虚函数表。

4) **gpiolib.c 是枢纽层** — 管理 chip 注册表、实现 consumer API、完成 dispatch 接线。理解它等于理解了整个设计的串联逻辑。

5) **N×M→N+M 解耦** — 中间层 gpiolib 将"设备数×芯片数"的代码量从乘法变为加法。芯片和设备越多，收益越显著。

6) **两家厂商实现**：vendor_a 用 DR_REG（数据寄存器，非原子读-改-写，模拟片内 GPIO 控制器）；vendor_b 用 BSRR（SET/CLR 双寄存器，原子单 store，模拟外扩 IO 扩展芯片）。

7) **厂商差异点**：vendor_a (base=0, ngpio=32) vs vendor_b (base=32, ngpio=16)；寄存器命名差异（DIR vs MODE, DR vs BSRR）；vendor_b 有 LOCK 寄存器解锁步骤。

8) **struct gpio_chip** = C 语言虚基类，5 个函数指针（request/free/direction_output/get/set）= 虚函数表。vendor 驱动填充这些指针后通过 gpiochip_add() 注册。

9) **struct gpio_desc** 是多态 dispatch 的钥匙 — (chip + offset) 对，避免每次调用遍历 chip 列表。

10) **演进关系**：ch15 用全局 platform_ops 指针（整个进程只有一套芯片实现）；ch16 改为按 chip 实例分组（多份 gpio_chip 共存，贴近真实 Linux 内核场景）。

11) **内核等价物**：gpio_chip.h → include/linux/gpio/driver.h；gpiolib.c → drivers/gpio/gpiolib.c；leds_gpio.c → drivers/leds/leds-gpio.c；vendor_a.c → drivers/gpio/gpio-mxc.c。

12) **项目文件清单**：gpio_chip.h, gpiolib.c, leds_gpio.c, main.c, gpio_vendor_a.c, gpio_vendor_b.c, Makefile, README.md。

13) **构建**：gcc -Wall -Wextra -std=c99，零 warning 零 error。`make` 编译，`make run` 编译+运行，`make clean` 清理。

14) **注释全面增强（2026-06-03）**：所有源文件添加了 Doxygen 字段文档、ASCII 架构图框、函数语义注释、调用链全景图。只改注释，未动一行代码逻辑。

15) **README.md 已创建**（2026-06-03）— 收录五层架构图、核心设计思想、调用链全景、文件职责表、本轮注释优化记录。

16) 每层只做一件事：main.c（系统集成者：知道有哪些芯片/设备），leds_gpio.c（驱动工程师：调 gpiod_set_value），gpiolib.c（子系统维护者：chip 管理+dispatch），gpio_chip.h（接口设计者：定义契约），vendor_*.c（厂商驱动工程师：寄存器操作）。
