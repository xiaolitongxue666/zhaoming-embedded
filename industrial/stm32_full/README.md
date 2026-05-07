# stm32_full — STM32 完整工程（工业级骨架）

对应附录 B。

完整的 STM32F407 工程，跑通全书 ch01-ch17 所有 OOP 抽象。功能：板上 4 颗 LED 演示 GPIO + PWM + I2C 三种子类混搭，应用层只看 `struct led_base *` 句柄，看不到子类完整类型，"换硬件不改应用"在三种维度同时演示。

工程组织参照真实工业项目的代码结构整理而成，保留教学需要的最小集合。

## 工业纪律

工程严格执行三层信息可见性差别：

| 层 | 看得到的内容 | 看不到的内容 |
|---|---|---|
| **应用层** (`Core/Src/main.c` / `mock/main_pc.c`) | `struct led_base *` 句柄 | 子类完整类型、ops 表、平台细节 |
| **驱动层** (`app/drivers/led/`) | `struct led_gpio` / `led_pwm` / `led_i2c`、`platform_xxx` 封装函数 | `struct platform_pin_ops`、寄存器、HAL |
| **平台层** (`app/platform/`、`app/platform/arch/board/`) | 所有 ops 表、寄存器、HAL | / |

跨边界的纪律：

- 驱动层 `#include "platform/platform_pin.h"` / `platform_pwm.h` / `platform_i2c.h` 调封装函数，**永远不**直接碰寄存器或 HAL
- 平台层框架 (`platform_pin.c` / `platform_pwm.c`) 维护 `static const struct platform_xxx_ops *_g_ops` 指针，子类通过 `platform_xxx_register(&xxx_ops)` 启动期填进来
- `platform_i2c.c` 升级到 bus + client 二层（一颗 MCU 一路 I2C 控制器底下挂多颗从设备），ops 挂在 `struct platform_i2c_bus_device` 实例上而不是全局指针，板级调 `platform_i2c_bus_register(bus, ops)` 完成挂载
- 字符串 pin 名（`"PA.5"` / `"PD.12"`）：调用方写的是字面字符串，看不到 port 索引或寄存器地址。换芯片只改 `pin_board.c` 的解码

## 工程结构

```
stm32_full/
├── README.md                                # 本文件
├── Makefile                                 # 双模 build (真机 + MOCK)
├── linker_stm32f407.ld                      # 链接脚本 (含 8 个 moduleExport 段)
├── startup_stm32f407xx.s                    # 启动汇编骨架
├── Core/                                    # CubeMX 风格的 MCU 启动入口
│   ├── Inc/main.h
│   └── Src/main.c                           # 真机 main: HAL_Init + SystemClock_Config + module_export_exec
├── app/                                     # 应用层 (工业代码风格)
│   ├── project_config.h                     # PLATFORM_OS / PLATFORM_HEAP_ENABLE 等开关
│   ├── platform/                            # 平台抽象层
│   │   ├── platform_def.h                   # 跨编译器宏 + platform_err_t + container_of
│   │   ├── platform_assert.h / .c           # platform_assert + 默认 handler
│   │   ├── platform_pin.h / .c              # PIN device framework (字符串名 + ops 分发)
│   │   ├── platform_pwm.h / .c              # PWM device framework (channel + duty 0-255)
│   │   ├── platform_i2c.h / .c              # I2C device framework (bus + client 二层 + master_xfer)
│   │   ├── platform_module_export.h / .c    # 8 级 INIT_xxx_EXPORT 机制 (ARMCC/IAR/GCC + MOCK)
│   │   └── arch/board/
│   │       ├── pin_board.c                  # STM32 pin 子类 (依赖 HAL)
│   │       ├── pwm_board.c                  # STM32 pwm 子类 (HAL_TIM_PWM_*)
│   │       └── i2c_board.c                  # STM32 i2c bus 子类 (HAL_I2C_Master_*)
│   ├── drivers/
│   │   └── led/
│   │       ├── led_base.h / .c              # 父类接口 + dispatch (on / off / set_brightness)
│   │       ├── led_gpio.h / .c              # GPIO LED 子类 (拉线点亮)
│   │       ├── led_pwm.h  / .c              # PWM  LED 子类 (亮度可调, 实现 set_brightness)
│   │       └── led_i2c.h  / .c              # I2C  LED 子类 (寄存器写)
│   └── environment_cfg/
│       ├── environment_export.h             # 4 颗 LED 句柄统一 extern
│       └── led_cfg.c                        # 4 颗 LED 实例 (GPIO + PWM + I2C 混搭) + INIT_ENV_EXPORT
└── mock/                                    # PC 模拟模式
    ├── main_pc.c                            # PC 主程序
    ├── pin_board_pc.c                       # PC pin 子类  (printf 模拟 GPIO)
    ├── pwm_board_pc.c                       # PC pwm 子类  (printf 模拟 PWM)
    └── i2c_board_pc.c                       # PC i2c 子类  (printf 模拟 I2C)
```

## 8 级初始化机制

`platform_module_export.h` 提供 8 个优先级宏：

```c
INIT_BOARD_EXPORT(fn)         /* Level 1: 板级初始化 (pin / pwm / i2c / clock) */
INIT_PREV_EXPORT(fn)          /* Level 2: 组件预初始化 (heap / sys) */
INIT_DEVICE_EXPORT(fn)        /* Level 3: 设备初始化 */
INIT_COMPONENT_EXPORT(fn)     /* Level 4: 组件初始化 (中间件) */
INIT_ENV_EXPORT(fn)           /* Level 5: 环境配置 (LED 实例装配 / 外设绑定) */
INIT_APP_EXPORT(fn)           /* Level 6: 应用初始化 */
INIT_SYSTEM_READY_EXPORT(fn)  /* Level 7: 系统就绪 */
UNIT_TEST_EXPORT(fn)          /* Level 8: 单元测试 (跟前 7 级独立) */
```

`platform_module_export_exec()` 启动期顺序跑 1-7 级。第 8 级独立由 `platform_unit_test_exec()` 跑。

跨编译器（ARMCC / IAR / GCC）通过 `__attribute__((section))` + linker 段实现。MOCK 模式下用 `__attribute__((constructor(N)))` 在 main 之前自动跑，应用层调 `platform_module_export_exec()` 是 nop，**调用形态完全一致**。

## 4 颗 LED 配置（演示 3 种子类混搭）

`app/environment_cfg/led_cfg.c` 装配 4 颗 LED 实例：

| 句柄 | 子类 | 资源 | set_brightness 行为 |
|---|---|---|---|
| `led_status` | `struct led_gpio` | PD.12, 高电平点亮 | no-op (走父类默认) |
| `led_dimmer` | `struct led_pwm`  | PWM ch0          | 真生效, duty 0-255 |
| `led_panel`  | `struct led_i2c`  | I2C 0x3C reg 0x00 | no-op |
| `led_alarm`  | `struct led_gpio` | PD.15, 高电平点亮 | no-op |

应用层 `#include "environment_cfg/environment_export.h"` 一次拿到全部，调用一致：

```c
led_on(led_status);                  /* GPIO 子类底下拉 PD.12 高 */
led_on(led_dimmer);                  /* PWM 子类底下使能通道 + 写 duty */
led_set_brightness(led_dimmer, 128); /* 父类 dispatch 到 PWM 子类生效 */
led_set_brightness(led_status, 128); /* 父类 dispatch, GPIO 子类未实现, no-op */
```

## 两种 build 模式

### PC 模拟模式（不依赖 STM32 HAL）

直接验证整套 OOP 抽象的业务逻辑。所有 GPIO/PWM/I2C 操作走 mock 板级实现打到 stdout：

```bash
cd industrial/stm32_full
make MOCK=1
./build/firmware-pc.exe       # Windows
./build/firmware-pc           # Linux/Mac
```

预期输出（节选）：

```
    [PIN] PD.12 mode -> OUTPUT
    [PIN] PD.12 <- LOW
    [PWM] ch0 disable
    [PWM] ch0 duty=0% (raw=0/255)
    [PIN] PD.15 mode -> OUTPUT
    [PIN] PD.15 <- LOW
=========================================
  stm32_full PC mock: 4 LED demo
  GPIO + PWM + I2C 三种子类混搭
=========================================

--- round 0 ---
[app] led_on(status)
    [PIN] PD.12 <- HIGH
[app] led_off(status)
    [PIN] PD.12 <- LOW
[app] led_on(dimmer)
    [PWM] ch0 enable
    [PWM] ch0 duty=100% (raw=255/255)
    [PWM] ch0 duty=50% (raw=128/255)
[app] led_off(dimmer)
    [PWM] ch0 duty=0% (raw=0/255)
    [PWM] ch0 disable
[app] led_on(panel)
    [I2C] addr=0x3c W len=2 data=00 01
[app] led_off(panel)
    [I2C] addr=0x3c W len=2 data=00 00
...
```

3 圈演示后退出，0 警告 0 错误。

### 真机模式（STM32F407 Discovery）

```bash
# 1. 装 arm-none-eabi-gcc 工具链
# 2. 用 STM32CubeMX 生成 STM32F407VGTx 工程, 把生成的 Drivers/ 目录拷贝到 vendor/
#    Drivers/STM32F4xx_HAL_Driver/  -> vendor/STM32F4xx_HAL_Driver/
#    Drivers/CMSIS/                  -> vendor/CMSIS/
# 3. 把 CubeMX 生成的 SystemClock_Config + MX_GPIO_Init 抄进 Core/Src/main.c
#    把 startup_stm32f407xx.s 替换为 CubeMX 生成版 (替换骨架)
# 4. make
make
make flash    # 用 st-flash 烧到 0x08000000
```

真机版三个板级文件 (`pin_board.c` / `pwm_board.c` / `i2c_board.c`) 全部齐备, 各自启动期 `INIT_BOARD_EXPORT` 注册到 `platform_pin / platform_pwm / platform_i2c` 框架. PWM 走 `HAL_TIM_PWM_Start/Stop` + `__HAL_TIM_SET_COMPARE`, I2C 走 `HAL_I2C_Master_Transmit/Receive`. CubeMX 生成的 `htim3 / htim4 / hi2c1` 句柄真机 main 提供, 板级文件 `extern` 引用.

## 跨芯片移植

`app/platform/arch/board/pin_board.c` 是 STM32F4 实现。换 H7/F1/L4/G0：

1. 复制 `pin_board.c` → `pin_board_stm32xx.c`，改 `#include "stm32xxxx_hal.h"`
2. 时钟使能宏 `__HAL_RCC_GPIOx_CLK_ENABLE()` 在不同芯片名字微差，改名字即可
3. 应用层、驱动层（`app/drivers/`）、平台框架（`app/platform/platform_pin.c` 等）一字不改

## 跨开发板 / 跨硬件

板子上 LED 引脚 / PWM 通道 / I2C 地址不一样时只改 `app/environment_cfg/led_cfg.c` 里的 4 行实例装配：

```c
led_gpio_init(&_led_status_inst, "status", "PD.12", true);   /* 改 pin 名 */
led_pwm_init (&_led_dimmer_inst, "dimmer", 0);                /* 改 channel */
led_i2c_init (&_led_panel_inst,  "panel",  &pc_i2c_bus0, 0x3C, 0x00);  /* bus + client_addr + reg */
led_gpio_init(&_led_alarm_inst,  "alarm",  "PD.15", true);
```

更换硬件方案 (例如把 dimmer 从 PWM 换成 I2C 灯条) 只需把 `led_pwm_init` 替成 `led_i2c_init`, 重新指定参数, 应用层 / 业务层 / 驱动层一字不动. 这就是"换硬件不改应用".

## 已知工程性细节

工程结构完整、抽象覆盖全。下面几点真机移植时按 CubeMX 生成版补全：

- `linker_stm32f407.ld` 中 `_Min_Heap_Size` / `_Min_Stack_Size` / `.preinit_array` / `.init_array` / `.ARM.exidx`
- `startup_stm32f407xx.s` 的 80+ 中断向量
- `Core/Src/main.c` 中 `SystemClock_Config` / `MX_GPIO_Init` / `MX_TIM3_Init` / `MX_TIM4_Init` / `MX_I2C1_Init` 的 HAL 调用细节 (用 CubeMX 把 TIM3 / TIM4 配 PWM 输出, I2C1 配 master, 把生成版抄进来)

这些都是 CubeMX 自动生成的工程性细节, 不影响 OOP 抽象的教学价值.
