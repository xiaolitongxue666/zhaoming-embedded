# 学习计划 · C 语言面向对象编程·嵌入式实战

在线书：[zhaochengbo.github.io/zhaoming-embedded](https://zhaochengbo.github.io/zhaoming-embedded/)

本文件与 `practice/` 目录同步更新。建议每章完成后在「总进度」中打勾，并在「完成日期」列填写实际日期。

---

## 学习原则

1. **先读再做**：读完对应在线章节后再打开 `practice/.../pc/`。
2. **先关参考**：手写阶段不要打开 `oop-in-c/code/` 同名目录。
3. **编译验收**：`gcc -Wall -Wextra` 0 警告，`./demo` 输出符合本章 README。
4. **再对比**：通过验收后再对照官方教学包，记录差异与疑问。

---

## 环境准备

| 步骤 | 说明 |
|------|------|
| 安装 GCC + make | 见 [`setup/README.md`](../setup/README.md) |
| 克隆本仓库 | 路径尽量使用纯英文，避免中文路径编译问题 |
| 验证 | `cd practice/01-封装/ch01-three-leds/pc && make && ./demo` |

---

## 总进度

在 `[ ]` 中改为 `[x]` 表示完成。

| 阶段 | 章节 | 进度 | 完成日期 |
|------|------|------|----------|
| 0 序曲 | 序曲 | [ ] | |
| 1 封装 | ch01 | [ ] | |
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
- 预热：运行官方 `oop-in-c/code/01-three-leds/pc` 的 demo，建立直观印象
- 说明：见 [`00-序曲/README.md`](00-序曲/README.md)

### 阶段 1 · 封装 ch01–ch05（建议每章 3–7 天）

| 章 | 练习目录 | 在线阅读 |
|----|----------|----------|
| ch01 | [`01-封装/ch01-three-leds/pc/`](01-封装/ch01-three-leds/pc/) | [三个 LED 三份代码](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/01-三个LED三份代码.html) |
| ch02 | [`01-封装/ch02-static-hiding/pc/`](01-封装/ch02-static-hiding/pc/) | [同事改了一行 · static](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/02-同事改了一行.html) |
| ch03 | [`01-封装/ch03-handwritten-class/pc/`](01-封装/ch03-handwritten-class/pc/) | [手搓 class](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/03-手搓class.html) |
| ch04 | [`01-封装/ch04-data-classification/pc/`](01-封装/ch04-data-classification/pc/) | [数据三级分类](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/04-数据归位.html) |
| ch05 | [`01-封装/ch05-hal-mapping/pc/`](01-封装/ch05-hal-mapping/pc/) | [HAL 映射](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/05-HAL映射.html) |

### 阶段 2 · 继承 ch06（建议 5–7 天）

| 章 | 练习目录 | 在线阅读 |
|----|----------|----------|
| ch06 | [`02-继承/ch06-inherit-pain/pc/`](02-继承/ch06-inherit-pain/pc/) | [代码一半重复](https://zhaochengbo.github.io/zhaoming-embedded/02-继承/06-代码一半重复.html) |

### 阶段 3 · 多态 ch07–ch11（建议每章 4–7 天）

| 章 | 练习目录 | 在线阅读 |
|----|----------|----------|
| ch07 | [`03-多态/ch07-function-pointer/pc/`](03-多态/ch07-function-pointer/pc/) | [写死的函数怎么换](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/07-写死的函数怎么换.html) |
| ch08 | [`03-多态/ch08-callback/pc/`](03-多态/ch08-callback/pc/) | [把号码给别人拨](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/08-把号码给别人拨.html) |
| ch09 | [`03-多态/ch09-ops-table/pc/`](03-多态/ch09-ops-table/pc/) | [ops 操作表](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/09-ops操作表.html) |
| ch10 | [`03-多态/ch10-vptr/pc/`](03-多态/ch10-vptr/pc/) | [ops 放进对象](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/10-ops放进对象.html) |
| ch11 | [`03-多态/ch11-polymorphism/pc/`](03-多态/ch11-polymorphism/pc/) | [多态完整图景](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/11-多态完整图景.html) |

### 阶段 4 · 工程威力 ch12–ch18（建议每章 5–10 天）

| 章 | 练习目录 | 在线阅读 |
|----|----------|----------|
| ch12 | [`04-工程威力/ch12-upcasting/pc/`](04-工程威力/ch12-upcasting/pc/) | [向上转型](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/12-向上转型.html) |
| ch13 | [`04-工程威力/ch13-container-of/pc/`](04-工程威力/ch13-container-of/pc/) | [container_of](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/13-container_of.html) |
| ch14 | [`04-工程威力/ch14-pure-virtual/pc/`](04-工程威力/ch14-pure-virtual/pc/) | [纯虚与抽象类](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/14-纯虚与抽象类.html) |
| ch15 | [`04-工程威力/ch15-platform/pc/`](04-工程威力/ch15-platform/pc/) | [Platform 抽象](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/15-Platform抽象.html) |
| ch16 | [`04-工程威力/ch16-linux-style/pc/`](04-工程威力/ch16-linux-style/pc/) | [Linux 不难](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/16-Linux不难.html) |
| ch17 | [`04-工程威力/ch17-initcall/pc/`](04-工程威力/ch17-initcall/pc/) | [initcall](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/17-initcall.html) |
| ch18 | [`04-工程威力/ch18-roadmap/pc/`](04-工程威力/ch18-roadmap/pc/) | [全书地图](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/18-全书地图.html) |

### 阶段 5 · 工业实战 + 附录（硬件可选）

- 阅读：[ch19 Zephyr](https://zhaochengbo.github.io/zhaoming-embedded/05-工业实战/19-主控案例.html) · 工程 [`industrial-zephyr/`](../industrial-zephyr/)
- 阅读：[ch20 Linux](https://zhaochengbo.github.io/zhaoming-embedded/05-工业实战/20-子控案例.html) · 工程 [`industrial-linux/`](../industrial-linux/)
- 说明：见 [`05-工业实战/README.md`](05-工业实战/README.md)

---

## 逐章任务卡片

### ch01 · 三个 LED 三份代码

- **概念**：重复代码痛点；`struct led` + `me` 指针；`platform_*` 胶水层
- **手写**：`led.h`、`led.c`（三实例共用一套 API）
- **验收**：三颗 LED（Pin 13/14/15）依次点亮，GPIO 日志来自 `platform_pc.c`
- **参考**：[`oop-in-c/code/01-three-leds/`](../oop-in-c/code/01-three-leds/)

### ch02 · static 与信息隐藏

- **概念**：`static` 文件内可见；`/* private */` 约定；读状态走 API
- **手写**：在 ch01 基础上加 `led_get_state`、内部 `static` 工具函数
- **验收**：外部无法误改内部状态（逻辑上通过 API 访问）
- **参考**：[`oop-in-c/code/02-static-hiding/`](../oop-in-c/code/02-static-hiding/)

### ch03 · 手搓 class

- **概念**：`模块_动作` 命名；句柄 + 操作函数；LED 与 motor 同套路
- **手写**：`led.h/c`、`motor.h/c`；`main.c` 演示两模块并列
- **验收**：`led_init` / `motor_init` 无符号冲突，行为符合书内示例
- **参考**：[`oop-in-c/code/03-handwritten-class/`](../oop-in-c/code/03-handwritten-class/)

### ch04 · 数据三级分类

- **概念**：实例 / 模块 / 全局数据归位；反面 `led_bad` 对比（可选）
- **手写**：按书重构数据归属；理清哪些字段进 `struct`
- **验收**：无多余全局可变状态驱动 LED 行为
- **参考**：[`oop-in-c/code/04-data-classification/`](../oop-in-c/code/04-data-classification/)

### ch05 · HAL 映射（偏阅读）

- **概念**：抽象接口 → 平台实现；对照 ST HAL 头文件分层
- **手写**：以阅读 + 笔记为主；可选在 `pc/` 写最小 platform 对照表
- **验收**：能画出应用 / 驱动 / HAL / 寄存器四层关系
- **参考**：[`oop-in-c/code/05-hal-mapping/`](../oop-in-c/code/05-hal-mapping/)

### ch06 · 继承痛点

- **概念**：`struct led_base` + 子类 `led_gpio` / `led_pwm`；子类 init 先调父类
- **手写**：`led_base.*`、`led_gpio.*`、`led_pwm.*`
- **验收**：子类复用父类字段布局，减少重复代码
- **参考**：[`oop-in-c/code/06-inherit-pain/`](../oop-in-c/code/06-inherit-pain/)

### ch07 · 函数指针入门

- **概念**：函数名 = 地址；`void (*fp)(int)` 换实现
- **手写**：`main.c` 中 `gpio_on` / `pwm_on` / `i2c_on` + 函数指针切换
- **验收**：切换 `fp` 后打印不同实现路径
- **参考**：[`oop-in-c/code/07-function-pointer/`](../oop-in-c/code/07-function-pointer/)

### ch08 · callback

- **概念**：把函数指针当参数传入；注册回调
- **手写**：`led.c` / `led.h` 带 callback 字段或注册接口
- **验收**：main 注册不同底层实现，LED API 行为一致
- **参考**：[`oop-in-c/code/08-callback/`](../oop-in-c/code/08-callback/)

### ch09 · ops 操作表

- **概念**：`struct led_ops` 表；`led_ops_gpio` / `led_ops_pwm` 实例
- **手写**：ops 表定义与填充，通过表调用
- **验收**：换表即换实现，main 不碰具体驱动函数名
- **参考**：[`oop-in-c/code/09-ops-table/`](../oop-in-c/code/09-ops-table/)

### ch10 · vptr 落地

- **概念**：ops 指针放进 `struct led_base`；dispatch 经 `ops->on`
- **手写**：base 含 `const struct led_ops *ops`
- **验收**：`led_on(led)` 内部走 `led->ops->on(led)`
- **参考**：[`oop-in-c/code/10-vptr/`](../oop-in-c/code/10-vptr/)

### ch11 · 多态完整图景

- **概念**：gpio / pwm / i2c 子类 + 统一 `led_on` 胶水
- **手写**：完整子类集 + 对外 API
- **验收**：同一句 `led_on` 驱动不同子类实例
- **参考**：[`oop-in-c/code/11-polymorphism/`](../oop-in-c/code/11-polymorphism/)

### ch12 · 向上转型

- **概念**：`struct led_base *` 指向任意子类；数组统一遍历
- **手写**：基类指针数组 + 循环 `led_on`
- **验收**：一个循环点亮 gpio / pwm / i2c 实例
- **参考**：[`oop-in-c/code/12-upcasting/`](../oop-in-c/code/12-upcasting/)

### ch13 · container_of

- **概念**：从成员指针反推容器；`offsetof` 公式
- **手写**：实现或使用 `container_of` 宏，从 `base` 找回子类
- **验收**：回调只收到 `base *` 时能调用子类特有逻辑
- **参考**：[`oop-in-c/code/13-container-of/`](../oop-in-c/code/13-container-of/)

### ch14 · 纯虚与抽象类

- **概念**：ops 某成员为 NULL / `-ENOSYS`；板级 init 表
- **手写**：抽象 ops + `sensor_board_init` 一类板级装配
- **验收**：未实现的 op 有明确失败路径
- **参考**：[`oop-in-c/code/14-pure-virtual/`](../oop-in-c/code/14-pure-virtual/)

### ch15 · Platform 抽象

- **概念**：四层架构；换平台只改 platform / 驱动
- **手写**：platform 初始化 + 驱动注册；应用层稳定
- **验收**：PC 与书中分层一致，应用 `.c` 不直接调 `printf` GPIO
- **参考**：[`oop-in-c/code/15-platform/`](../oop-in-c/code/15-platform/)

### ch16 · Linux 风格

- **概念**：tab 缩进；`struct device`；gpiolib 风格封装
- **手写**：按书整理为 Linux 内核命名与分层习惯
- **验收**：代码风格与 `oop-in-c` ch16 demo 同级
- **参考**：[`oop-in-c/code/16-linux-style/`](../oop-in-c/code/16-linux-style/)

### ch17 · initcall

- **概念**：链接段收集 init；`MODULE_INIT` 宏
- **手写**：`initcall.h` + 多个 `drv_*.c` 自动注册
- **验收**：main 不显式调用各驱动 init，遍历段表即可
- **参考**：[`oop-in-c/code/17-initcall/`](../oop-in-c/code/17-initcall/)

### ch18 · 全书地图

- **概念**：一颗 LED 的演化五阶段回顾
- **手写**：`main.c` 串联打印或调用各阶段代表性 API
- **验收**：运行 demo 能复述从 ch01 到 ch17 的演进
- **参考**：[`oop-in-c/code/18-roadmap/`](../oop-in-c/code/18-roadmap/)

### ch19 · Zephyr 实战（无 practice/pc）

- **阅读**：[主控案例](https://zhaochengbo.github.io/zhaoming-embedded/05-工业实战/19-主控案例.html)
- **动手**：[`industrial-zephyr/`](../industrial-zephyr/)，`west build -b stm32f4_disco -p auto -- -DDEMO=1`
- **附录**：[附录 B](https://zhaochengbo.github.io/zhaoming-embedded/附录/B-STM32完整工程.html)

### ch20 · Linux 实战（无 practice/pc）

- **阅读**：[子控案例](https://zhaochengbo.github.io/zhaoming-embedded/05-工业实战/20-子控案例.html)
- **动手**：[`industrial-linux/ch20-leds-status/`](../industrial-linux/ch20-leds-status/)
- **附录**：[附录 C](https://zhaochengbo.github.io/zhaoming-embedded/附录/C-Linux完整工程.html)

---

## 无硬件时的备选路径

与书附录 D.4 一致：

1. ch01–ch18：只做 `practice/**/pc` + 对照 `oop-in-c/code`
2. ch19：读 `industrial-zephyr/src` + Zephyr 上游 driver 源码
3. ch20：读 `industrial-linux` + 书内 `leds-status.c` 片段

---

## 配套资源

| 资源 | 链接 |
|------|------|
| 在线书 | https://zhaochengbo.github.io/zhaoming-embedded/ |
| 官方教学包 | [`oop-in-c/code/`](../oop-in-c/code/) |
| 代码索引 | [`book/附录/D-配套代码索引.md`](../book/附录/D-配套代码索引.md) |
| 环境配置 | [`setup/README.md`](../setup/README.md) |
| B 站 | 搜「兆鸣嵌入式」 |
