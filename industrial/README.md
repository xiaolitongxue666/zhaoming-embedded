# industrial · 完整工程

这本书附录 B / C 配套的完整工业级工程，跑通 ch01-ch17 所有 OOP 抽象。

| 目录 | 对应 | 平台 | 状态 |
|---|---|---|---|
| `stm32_full/` | 附录 B | STM32F407 / Cortex-M4F · CubeMX HAL | PC mock 跑通；真机依赖 CubeMX 生成的 HAL 库 |
| `linux_full/` | 附录 C | ARM64 SBC（树莓派 / 香橙派）· 用户态 | 计划 |

## 跟 oop-in-c/ 教学代码的关系

`oop-in-c/code/<chapter>/` 是教学版（小、清晰、聚焦每章一个概念）。`industrial/` 是工业版（多文件分层、跨编译器宏、`platform_err_t` 错误码、8 级 initcall、`platform_assert` 校验、Doxygen 注释）。

两者抽象同构：教学版每一招（struct + me / static + 不透明指针 / 函数指针 / ops 表 / vptr / container_of / ...）在工业版里都有对应位置，附录 B § B.5 / 附录 C § C.5 给出全表对应。

## 学习路径

1. 把 `oop-in-c/code/01-three-leds/` 到 `oop-in-c/code/17-initcall/` 教学版跑一遍，看清每个概念怎么演化出来
2. 读附录 B 章节正文，理解工程分层
3. `cd industrial/stm32_full && make MOCK=1` 跑 PC mock，看整套抽象在完整工程里是怎么协作的
4. 读附录 C，了解同套抽象在 Linux 用户态的形态
5. 拿到真板（STM32F407 Discovery / 树莓派）后按附录 B / C 的步骤上真机

## 工业级骨架的核心要素

附录 B 工程演示的"工业级"包括：

- **跨编译器**（ARMCC / IAR / GCC）的 attribute 宏统一在 `platform_def.h`
- **错误码** `platform_err_t` 全工程统一，`goto exit` 风格收拢错误处理
- **8 级 initcall** (`INIT_BOARD_EXPORT` 到 `UNIT_TEST_EXPORT`)，启动期自动跑
- **三层信息可见性**：应用层只见 `led_base_t *`、驱动层见子类完整类型、平台层见 ops 表
- **PIN 字符串名解析**（`"PA.5"` / `"PD.12"` / `"PI.14"`）让上层永远不碰寄存器
- **`platform_assert`** 工业级运行时校验

## 法律声明

这些代码是这本书的教学样本，可阅读、可学习、可改写。不构成任何工业设备或控制系统的可用驱动。不附带任何形式的担保（按 [MIT License](../LICENSE) 发布）。实际工业产品请走完整的行业认证流程。
