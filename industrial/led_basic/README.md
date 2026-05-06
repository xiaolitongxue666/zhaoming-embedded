# led_basic — 工业项目 LED 驱动最小骨架

这一份代码是《C 语言面向对象编程·嵌入式实战》第 19 章 19.1 节正文的配套骨架。

## 来源

来自工业控制板真实项目。这里抽出来的部分是 LED 驱动最小完整骨架，做了脱敏处理：去掉公司标识、去掉具体业务领域信息，只保留对 OOP 教学有意义的接口与实现。

完整工业项目约 80K 行 C 代码，本目录只取 LED 驱动这一个最简单的子集——它最能直观体现"基类 + ops 表 + 子类"三件套在工业代码里的真实形态。

## 文件清单

| 文件 | 角色 |
|---|---|
| `led_base.h` | 基类接口：`struct led_base_ops` + `led_base_t` + `led_on / led_off` 公开函数声明 |
| `led_base.c` | 基类派发实现：`led_on(me)` 内部走 `me->ops->led_on(me)` |
| `led_gpio.h` | GPIO 子类接口：`led_gpio_t { base, pin_num, light_level }` + `led_gpio_init` |
| `led_gpio.c` | GPIO 子类实现：`led_gpio_on / led_gpio_off` + 静态 `ops` 表 + 构造函数 |

## 跟章节正文的对应

19.1 节正文里贴出的每一段 LED 代码，在本目录都有字节级对应：

| 章节正文位置 | 本目录文件 |
|---|---|
| 19.1 节"挂号单一路推到工业代码会怎样"代码块（基类 ops 表 + `led_base_t`） | `led_base.h` |
| 19.1 节"`led_base.c` 实现就两行"代码块 | `led_base.c` |
| 19.1 节"GPIO 类型的子类"代码块（`led_gpio_t`） | `led_gpio.h` |
| 19.1 节"子类实现"代码块（`led_gpio_on / off / static ops / led_gpio_init`） | `led_gpio.c` |

## 这份代码不能直接 build

本目录依赖工业项目的 Platform 层（`platform_pin.h` / `platform_def.h`）。完整可 build 的等价骨架请看 [`industrial/stm32_full/`](../stm32_full/) 和 [`industrial/linux_full/`](../linux_full/) 两个完整工程。

## 法律声明

按 [MIT License](../../LICENSE) 发布。代码可阅读、可改写、可作为教学样本。不构成任何工业设备或控制系统的可用驱动；不附带任何形式的担保。实际工业产品请走完整的行业认证流程。
