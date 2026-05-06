# motor_24vfuncs — 工业项目电机驱动接口骨架

这一份代码是《C 语言面向对象编程·嵌入式实战》第 20 章 20.1 / 20.2 / 20.3 / 20.4 节正文的配套骨架。

## 来源

来自工业控制板真实项目。这块板上要同时驱动两路完全不同的电机：

- 一路水平方向：ACM 伺服电机，通过 UART 走厂家私有命令协议
- 一路垂直方向：IFX007T H 桥驱动的直流电机

两路电机能力差距巨大（伺服是 24 个虚方法的 ops 表，直流是 3 个），但是应用层接口风格完全统一。这就是第 20 章想讲透的事。

## 文件清单

### 水平电机 h_motor（20.1 / 20.2 / 20.4 节）

| 文件 | 角色 |
|---|---|
| `h_motor_base.h` | 24 个虚方法 ops 表 + 三种回调 + 应用层封装函数声明 |
| `h_motor_base.c` | 封装函数实现：每个函数体内部走 `me->ops->xxx(me, ...)` dispatch |
| `h_motor_acm.h` | ACM 伺服子类 struct + 构造参数 |
| `h_motor_acm.c` | 教学骨架（见下面"骨架与脱敏"说明） |

### 垂直电机 v_motor（20.3 节）

| 文件 | 角色 |
|---|---|
| `v_motor_base.h` | 3 虚方法 ops 表 + 2 回调，纯虚接口在基类层用 `platform_assert` 卡死 |
| `v_motor_base.c` | 封装函数实现 + 状态变化回调派发 + 派发前对纯虚 ops 的 `platform_assert` 校验 |
| `v_motor_ifx007t.h` | IFX007T H 桥子类 struct（PWM 设备 + sleep / 减速区限位 / IS 故障检测 ADC 通道） |
| `v_motor_ifx007t.c` | IFX007T 子类实现：双路 PWM + 软启动 + 加减速曲线 + 减速区慢爬 + IS 引脚电流采样 EMA 滤波 + 三层超时保护，完整产品级版本，跑在 work_thread 上下文里非阻塞 |

ch20 § 20.3 正文里"几行 GPIO 操作"是教学描述用的最小内核——把 OOP 抽象骨架（3 ops + 2 回调）讲清楚就够了。仓库里这一份 `v_motor_ifx007t.c` 是产品级版本，多出来的软启动 / 加减速 / 限位 / 电流故障保护这些工程细节并没有改变 OOP 骨架，子类对外仍然只暴露 motor_stop / motor_move / fault_clear 三个 ops 函数指针，应用层调用代码一行不动。两份代码的关系跟 ch19 主控板 LED 教学版 vs 工业版的关系一样：教学版讲清楚抽象骨架，工业版把骨架在产品级现实里跑通。

## 关于 h_motor_acm.c

`h_motor_acm.c` 完整 1439 行，包含：

- 24 个虚方法的完整实现
- 厂家私有协议帧组装（命令字节、参数字节序、帧头帧尾）
- CRC 校验代码
- 厂家私有寄存器地址（`ACM_REG_ACTUAL_POSITION` / `ACM_REG_PEAK_CURRENT_LIMIT` / `ACM_REG_I2T_RUNNING_SUM` 等）
- work_thread 命令分发逻辑
- rx_thread packet parser

脱敏只动公司信息：

- 厂家名清空，所有具体厂家代号一律改成行业通用名"ACM 伺服厂家"
- 文件头 `@author` / `Copyright` 行替换为 `SPDX-License-Identifier: MIT`
- include 路径从工业项目里的子目录形式（`platform/platform_pin.h`）改为本目录平铺形式（`platform_pin.h`）

寄存器地址、协议帧结构、虚方法实现、工作线程逻辑全部保留——这些不是隐私，就是一颗 ACM 伺服电机要驱动起来的工程样本。

## 跟章节正文的对应

| 章节正文位置 | 本目录文件 |
|---|---|
| 20.1 节"24 个虚方法的 ops 表"代码块（24 函数指针） | `h_motor_base.h` |
| 20.1 节"节选自 `h_motor_acm.c` 内部"代码块（`acm_velocity_set` 等 3 个示例虚方法 + `acm_work_t`） | `h_motor_acm.c` |
| 20.2 节"节选自 `app/drivers/h_motor/h_motor_acm.h`"代码块（含 `base` 字段、消息队列 / 工作线程、按键库、性能监控字段、`i2t_reg_unsupported` 标志） | `h_motor_acm.h` |
| 20.4 节"三种回调"代码块（status_cb_t / cur_pos_cb_t / err_cb_t） | `h_motor_base.h` |
| 20.3 节"垂直电机基类"代码块（3 虚方法 ops 表 + 2 回调） | `v_motor_base.h` |
| 20.3 节"IFX007T 子类"代码块（4 个引脚 struct） | `v_motor_ifx007t.h` |
| 20.3 节"`motor_move(UP)` 在子类里就是把 ctrl_1 拉高..." | `v_motor_ifx007t.c` |

## 这份代码不能直接 build

本目录依赖：
- 工业项目 Platform 层（`platform_def.h` / `platform_pin.h` / `platform_device.h` / `platform_assert.h`）
- CMSIS-RTOS2（`cmsis_os2.h`）
- MultiButton 按键库
- EasyLogger 日志（`elog.h`）

完整可 build 的简化版骨架请看 [`industrial/stm32_full/`](../stm32_full/) 和 [`industrial/linux_full/`](../linux_full/)（它们只演示 LED + Platform + initcall，不包含电机）。

## 法律声明

按 [MIT License](../../LICENSE) 发布。代码可阅读、可改写、可作为教学样本。不构成任何工业设备或控制系统的可用驱动；不附带任何形式的担保。实际工业产品请走完整的行业认证流程。
