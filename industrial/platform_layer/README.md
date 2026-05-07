# platform_layer — 工业项目 Platform 抽象层骨架

这一份代码是《C 语言面向对象编程·嵌入式实战》第 19 章 19.6 节、第 20 章 20.6 节正文的配套骨架。

## 这是裸机 STM32 项目专用·跟 Linux 项目无关

这一份 platform 抽象层（platform_pin / platform_pwm / platform_i2c / platform_uart / platform_spi / platform_module_export 等）**只针对裸机 STM32 / 简单 RTOS（FreeRTOS 单纯 kernel）这种没有现成设备模型的环境**。

**Linux 用户态 / Linux 内核态 / Zephyr / RT-Thread / NuttX** 这些环境内核已经把 platform 抽象做完了·应用层再套一层就是过度封装。具体看：

- ch15 §15.15 "什么时候不要 Platform 层"（建立判断标准）
- ch16 §16.13 "不只是 Linux：Zephyr / RT-Thread 也是同款"（横向扩展）
- 附录 C §C.0 "为什么这一份工程没有 platform 层"（Linux 用户态实战）

附录 B `industrial/stm32_full/` 是这一层 platform 抽象的"必须自抽"实战；附录 C `industrial/linux_full/` 是"内核做完别再抽"的对照实战·两个工程合起来覆盖工程判断力的完整光谱。

## 来源

来自工业控制板真实项目。Platform 层是把"换 MCU 不改应用"这件事真正做成的关键：所有驱动只调 `platform_xxx_*` 接口，每家 MCU 的 HAL 差异都在这一层吸收掉。

## 文件清单

| 文件 | 角色 | 章节正文对应 |
|---|---|---|
| `platform_module_export.h` | 7 级 initcall 宏定义（`INIT_BOARD_EXPORT` 到 `INIT_SYSTEM_READY_EXPORT`），跨 ARMCC / IAR / GCC 三大编译器 | 第 19 章 19.6 节"`INIT_xxx_EXPORT` 宏" |
| `platform_module_export.c` | initcall 调度的 7 级 `MODULE_EXPORT_EXEC` 循环，跨编译器实现 | 第 19 章 19.6 节"GCC 工具链版本宏展开 + 7 级遍历调用" |
| `platform_uart.h` | UART 抽象层接口：`platform_uart_dev_t` + `platform_uart_ops_t` + `platform_hw_uart_register`，第一个字段嵌入 `struct platform_device parent`（两层继承） | 第 20 章 20.6 节"UART 抽象的接口" |
| `platform_i2c.h` | I²C 抽象层接口：`platform_i2c_msg_t` 数组式 transfer 接口（同 Linux 内核 `i2c_transfer`），`platform_i2c_bus_device_t` + `platform_i2c_client_t` | 第 20 章 20.6 节"I²C 抽象用 message 数组的形式" |
| `platform_spi.h` | SPI 抽象层接口：`platform_spi_device_t` 表示具体片选下的设备 + `platform_spi_ops_t` 总线 ops 表 | 第 20 章 20.6 节"SPI 抽象用 `platform_spi_device_t`" |
| `platform_def.h` | 跨编译器宏定义 + `platform_err_t` 错误码枚举 + `container_of` / `offsetof` 实现 | 全书反复用到 |

## 跟章节正文的对应

第 19 章 19.6 节正文里贴出的 7 级 `INIT_xxx_EXPORT` 宏定义代码块和 GCC 版本的 `MODULE_EXPORT_EXEC` 循环，跟这里的 `platform_module_export.h` / `platform_module_export.c` 一字不差。

第 20 章 20.6 节正文里贴出的 `platform_uart_configure_t` / `platform_uart_dev_t` / `platform_uart_ops_t` / `platform_hw_uart_register`，跟 `platform_uart.h` 一字不差。20.6 节贴的 `platform_i2c_msg_t` 和 `platform_i2c_transfer` 接口跟 `platform_i2c.h` 一字不差。

## 这份代码不能直接 build

本目录依赖 `platform_device.h` / `cmsis_os.h`，它们在原工程里和这些头文件配套；本仓库的可 build 完整骨架在 [`industrial/stm32_full/`](../stm32_full/) 和 [`industrial/linux_full/`](../linux_full/)。

## 法律声明

按 [MIT License](../../LICENSE) 发布。代码可阅读、可改写、可作为教学样本。不构成任何工业设备或控制系统的可用驱动；不附带任何形式的担保。实际工业产品请走完整的行业认证流程。
