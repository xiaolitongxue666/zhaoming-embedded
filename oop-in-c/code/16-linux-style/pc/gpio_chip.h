/* SPDX-License-Identifier: MIT */
/*
 * gpio_chip.h - 芯片抽象层：gpio_chip 结构体 & consumer API 声明
 *
 * ┌─ 设计定位 ────────────────────────────────────────────┐
 * │ 这是整个 GPIO 子系统的接口核心。定义两层抽象：          │
 * │                                                        │
 * │ (1) struct gpio_chip ── "芯片控制器抽象"               │
 * │   每家厂商的 GPIO 控制器都抽象成这个结构体。关键字段    │
 * │   是 5 个函数指针（相当于 C++ 的虚函数表），厂商驱动    │
 * │   往这些指针挂自己的寄存器操作函数。                    │
 * │                                                        │
 * │ (2) struct gpio_desc ── "引脚描述符"                   │
 * │   一个 (chip + offset) 对，代表某个芯片上的某个引脚。   │
 * │   consumer API 通过 desc 反查 chip，调对应函数。       │
 * │   这是多态 dispatch 的"钥匙"。                         │
 * │                                                        │
 * │ 真实内核版：include/linux/gpio/driver.h L415+          │
 * │ 字段比这里多（中断、复用、pinctrl、debugfs 等），      │
 * │ 但核心骨架 —— gpio_chip + ops 表 —— 完全一致。         │
 * └────────────────────────────────────────────────────────┘
 *
 * ┌─ 与前章演进关系 ──────────────────────────────────────┐
 * │ ch15 的 platform_ops 是"全局一份函数指针"，整个进程    │
 * │ 同一时刻只有一套芯片实现。ch16 改为按 chip 实例分组：  │
 * │ 每个 chip 有自己的 ops 表，desc->gc 反查 dispatch。    │
 * │ 这才贴近真实内核 —— 同一颗 SoC 上可能同时有片内 GPIO   │
 * │ 控制器 + 外扩 IO 扩展芯片，多份 gpio_chip 共存。       │
 * │ 参见 ch16 §16.3 四层架构、§16.5 真实内核代码          │
 * └────────────────────────────────────────────────────────┘
 *
 * ┌─ 五层架构层级关系（从下到上） ────────────────────────┐
 * │                                                        │
 * │  第5层  初始化/演示层  (main.c)                         │
 * │  模拟内核启动 → probe → 获取 desc → 操作              │
 * │                                                        │
 * │  第4层  设备驱动层    (leds_gpio.c)                     │
 * │  leds-gpio 驱动，调 gpiod_set_value                    │
 * │  不关心底层是哪家芯片                                   │
 * │                                                        │
 * │  第3层  核心框架层    (gpiolib.c)                       │
 * │  注册 chip 管理列表 + consumer API                     │
 * │  gpiod_set_value → desc->gc->set 多态分发              │
 * │                                                        │
 * │  第2层  芯片抽象接口  (gpio_chip.h)  ← 本文件           │
 * │  定义 struct gpio_chip（5个函数指针 ops 表）            │
 * │  定义 struct gpio_desc（chip + offset 对）              │
 * │                                                        │
 * │  第1层  硬件实现层                                      │
 * │  ├─ gpio_vendor_a.c (DR_REG 数据寄存器风格)             │
 * │  └─ gpio_vendor_b.c (SET/CLR 寄存器风格)               │
 * │                                                        │
 * │  核心设计思想：                                          │
 * │  ① 多态 dispatch — desc->gc->set 同一行代码，          │
 * │     根据 desc->gc 指向不同 chip 实例，走不同实现        │
 * │  ② N×M→N+M — 中间层把乘法变加法                      │
 * │     无抽象层：M 个驱动 × N 家芯片 → M×N 份重复代码    │
 * │       每加一家芯片，LED、按键、LCD 所有驱动都得改      │
 * │     有 gpiolib：M 个驱动 + N 家芯片驱动 → M+N          │
 * │       加芯片 = 写 1 个 vendor 文件填 ops 表           │
 * │       已有驱动一行不改，自动支持新芯片                  │
 * │     芯片越多、设备越多，M+N 和 M×N 的差距越悬殊       │
 * │  ③ 每层只做一件事，层间通过接口对话                    │
 * └────────────────────────────────────────────────────────┘
 */

#ifndef GPIO_CHIP_H
#define GPIO_CHIP_H

#include <stdint.h>
#include <stdbool.h>

/**
 * struct gpio_chip - 抽象一个 GPIO 控制器芯片
 *
 * @label:    芯片标识字符串。consumer 通过 label 查找目标芯片。
 *            相当于驱动设备树中的 compatible 匹配。
 * @base:     该芯片在"全局 GPIO 编号空间"中的起始编号。
 *            内核中用于全局 gpio_to_irq 等整数查找接口，
 *            这里保留以模拟内核形态，实际传 desc 时不走 base。
 *            举例：vendor_a (base=0, ngpio=32) 占全局编号 0-31，
 *            vendor_b (base=32, ngpio=16) 占 32-47，两片互不重叠。
 *            这套按 base 分割的全局编号是内核 legacy 接口
 *            （已被 GPIO descriptor 取代），保留是为了对齐
 *            真实内核 include/linux/gpio/driver.h 的结构。
 * @ngpio:    该芯片提供的 GPIO 引脚数量。
 *            desc->offset 的取值范围就是 0 .. ngpio-1。
 *            base 和 ngpio 共同描述了这家芯片在全局地址空间
 *            中的位置和容量：vendor_a 从 0 号开始占 32 个脚，
 *            vendor_b 从 32 号开始占 16 个脚。
 * @request:  请求（申请）一个引脚。类似内核 gpiochip_request_irq。
 *            厂商实现中通常涉及 PORT_EN / LOCK 等寄存器操作。
 * @free:     释放一个引脚。与 request 配对。
 * @direction_output: 设置引脚为输出模式并赋初值。
 *            内核中还有 direction_input，这里只演示输出场景。
 * @get:      读取引脚当前电平。返回 0 或 1。
 * @set:      设置引脚输出电平。多态 dispatch 的关键落点。
 *            gpiod_set_value 最终调的就是这个指针。
 * @driver_data: 厂商私有数据的挂载点。vendor 驱动可以在此
 *            挂自己的上下文结构体（寄存器基址、时钟句柄等），
 *            相当于 Linux 内核的 gpio_chip->data。
 *
 * ┌─ 多态 dispatch 机制 ─────────────────────────────────┐
 * │ struct gpio_chip 就是一个手动实现的 C 语言虚基类。    │
 * │ —— 或者说，它就是 C 语言中的"父类"。                   │
 * │                                                        │
 * │ gpio_chip 定义接口契约：label + base/ngpio 描述芯片    │
 * │ 身份和容量，5 个函数指针 = 5 个虚方法。各家 vendor    │
 * │ 实例化一个 gpio_chip，填入自己的实现函数（= 子类      │
 * │ 重写虚函数），然后调用 gpiochip_add() 注册到框架。     │
 * │                                                        │
 * │ 这不是 C++ 的继承（没有派生类语法），而是"接口继承"   │
 * │ —— 一个接口（struct 契约），多种实现（各家 vendor     │
 * │ 填充的函数指针）。这是 C 语言实现 OOP 最经典的方式。  │
 * │                                                        │
 * │ gpiolib.c 不感知具体实现，只通过 desc->gc 调函数指针。 │
 * │ 这就是 ch11 多态在 Linux GPIO 中的真实形态。           │
 * └────────────────────────────────────────────────────────┘
 */
struct gpio_chip {
	const char *label;
	uint32_t    base;
	uint32_t    ngpio;

	/* --- ops 表：5 个函数指针，各家厂商挂自己的实现 --- */
	int  (*request)(struct gpio_chip *gc, unsigned int offset);
	void (*free)(struct gpio_chip *gc, unsigned int offset);
	int  (*direction_output)(struct gpio_chip *gc,
				 unsigned int offset, int value);
	int  (*get)(struct gpio_chip *gc, unsigned int offset);
	void (*set)(struct gpio_chip *gc, unsigned int offset, int value);

	void *driver_data;
};

/**
 * struct gpio_desc - GPIO 引脚描述符
 * @gc:     指向该引脚所属的 chip 实例。多态 dispatch 的入口。
 * @offset: 在该 chip 内部引脚编号（0 .. ngpio-1）。
 *
 * ┌─ 为什么需要 desc 而不直接用 (chip, offset) 两个参数？ ┐
 * │ 内核 consumer 在 open 阶段通过 device tree 完成一次    │
 * │ 查找（得 desc），之后所有操作都传 desc 而非重复查找。  │
 * │ 这样 gpiod_set_value(desc) 一次就能完成 dispatch，     │
 * │ 省去每次调用都遍历 chip 列表的开销，也方便引用计数。   │
 * └────────────────────────────────────────────────────────┘
 */
struct gpio_desc {
	struct gpio_chip *gc;
	unsigned int      offset;
};

/* --- Consumer API（简化版内核消费者接口） --- */

/**
 * gpiod_set_value - 设置 GPIO 引脚输出电平
 * @desc:  目标引脚描述符（不能为 NULL，由调用方保证）
 * @value: 输出电平（0 = 低，非 0 = 高）
 *
 * 内部通过 desc->gc->set 进行多态 dispatch。
 */
void gpiod_set_value(struct gpio_desc *desc, int value);

/**
 * gpiod_get_value - 读取 GPIO 引脚输入电平
 * @desc:  目标引脚描述符
 * Return: 0（低电平），1（高电平），-1（desc 无效）
 */
int  gpiod_get_value(struct gpio_desc *desc);

/* --- Provider API（芯片驱动注册接口） --- */

/**
 * gpiochip_add - 注册一个 gpio_chip 到子系统
 * @gc:  已填好 ops 表字段的 chip 实例（静态分配或动态分配均可）
 * Return: 0 成功，-1 失败（chip 表满）
 *
 * 真实内核中叫 gpiochip_add_data，还接收一个 void *data
 * 自动挂到 gc->driver_data。这里简化了。
 */
int gpiochip_add(struct gpio_chip *gc);

/**
 * gpio_get_desc - 通过 chip label + offset 获取引脚描述符
 * @chip_label: chip 的 label 字符串
 * @offset:     芯片内部引脚编号
 * Return: 动态分配的 gpio_desc 指针，调用方负责 free
 *
 * 真实内核中 consumer 通过 device tree 节点获取 desc，
 * 这里用 label 字符串作为查找 key，简化演示。
 */
struct gpio_desc *gpio_get_desc(const char *chip_label, unsigned int offset);

#endif /* GPIO_CHIP_H */
