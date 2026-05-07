# led_basic — 工业项目 LED 驱动最小骨架

这一份代码是《C 语言面向对象编程·嵌入式实战》第 19 章 19.1 节正文的配套骨架。

## 来源

来自工业控制板真实项目。这里抽出来的部分是 LED 驱动最小完整骨架，做了脱敏处理：去掉公司标识、去掉具体业务领域信息，只保留对 OOP 教学有意义的接口与实现。

完整工业项目约 80K 行 C 代码，本目录只取 LED 驱动这一个最简单的子集。它最能直观体现"基类 + ops 表 + 子类"三件套在工业代码里的真实形态。

## 文件清单

### 父类层

| 文件 | 角色 |
|---|---|
| `led_base.h` | 基类接口：`struct led_ops` + `struct led_base` + 公开函数声明（`led_on / led_off / led_set_brightness`） |
| `led_base.c` | 基类派发实现：`led_on(me)` 内部走 `me->ops->on(me)`，含三层 NULL 校验 |

### 子类层（三种硬件实现，同一组应用层接口）

| 文件 | 角色 |
|---|---|
| `led_gpio.h` / `led_gpio.c` | GPIO 拉线子类：`struct led_gpio { base, pin, active_high }`，不实现 set_brightness |
| `led_pwm.h` / `led_pwm.c`   | PWM 调亮度子类：`struct led_pwm { base, channel, brightness }`，实现 set_brightness |
| `led_i2c.h` / `led_i2c.c`   | I²C 控制子类：`struct led_i2c { base, client (bus + client_addr), reg, val_on, val_off }`，不实现 set_brightness |

三个子类共用一份 `struct led_base` 父类接口。应用层拿到的永远是 `struct led_base *` 句柄，调 `led_on / led_off / led_set_brightness` 三个 API 即可，看不到具体实现是 GPIO 拉线、PWM 调速还是 I²C 寄存器写。这就是第 19 章 19.1 节"换硬件不改应用"在工业代码里的最小完整形态。

## 跟章节正文的对应

19.1 节正文里贴出的每一段 LED 代码，在本目录都有逐节对应（接口、字段、函数签名一字不差，注释和缩进风格按工业代码规范展开）：

| 章节正文位置 | 本目录文件 |
|---|---|
| 19.1 节"挂号单一路推到工业代码会怎样"代码块（基类 ops 表 + `struct led_base`） | `led_base.h` |
| 19.1 节"`led_base.c` 实现就两行"代码块 | `led_base.c` |
| 19.1 节"GPIO 类型的子类"代码块（`struct led_gpio`） | `led_gpio.h` |
| 19.1 节"子类实现"代码块（`_led_gpio_on / _off / static ops / led_gpio_init`） | `led_gpio.c` |
| 19.1 节"PWM 子类追加 set_brightness"延伸（同基类指针下挂调亮度子类） | `led_pwm.h` / `led_pwm.c` |
| 19.1 节"I²C 子类换硬件不改应用"延伸（同基类指针下挂总线子类） | `led_i2c.h` / `led_i2c.c` |

## 这份代码不能直接 build

本目录依赖工业项目的 Platform 层（`platform_pin.h` / `platform_pwm.h` / `platform_i2c.h` / `platform_def.h` / `platform_assert.h`）。完整可 build 的等价骨架请看 [`industrial/stm32_full/`](../stm32_full/) 和 [`industrial/linux_full/`](../linux_full/) 两个完整工程。

## 法律声明

按 [MIT License](../../LICENSE) 发布。代码可阅读、可改写、可作为教学样本。不构成任何工业设备或控制系统的可用驱动；不附带任何形式的担保。实际工业产品请走完整的行业认证流程。
