# stm32_full — STM32 完整工程（工业级骨架）

对应附录 B。

完整的 STM32F407 工程，跑通全书 ch01-ch17 所有 OOP 抽象。功能：板上 4 颗 LED 流水灯（PD.12 / PD.13 / PD.14 / PD.15）。

工程组织参照真实工业项目（同款 STM32H7 控制板）的代码结构整理而成，脱敏后保留教学需要的最小集合。

## 工业纪律

工程严格执行三层信息可见性差别：

| 层 | 看得到的内容 | 看不到的内容 |
|---|---|---|
| **应用层** (`Core/Src/main.c` / `mock/main_pc.c`) | `led_base_t *` 句柄 | 子类完整类型、ops 表、平台细节 |
| **驱动层** (`app/drivers/led/`) | `led_gpio_t`、`platform_pin_xxx()` 封装函数 | `platform_pin_ops_t`、寄存器、HAL |
| **平台层** (`app/platform/`、`app/platform/arch/board/`) | 所有 ops 表、寄存器、HAL | / |

跨边界的纪律：

- 驱动层 `#include "platform/platform_pin.h"` 调封装函数，**永远不**直接碰 GPIO 寄存器或 HAL
- 平台层框架 `platform_pin.c` 维护 `static const platform_pin_ops_t *_g_ops` 指针，子类通过 `platform_pin_register(&xxx_ops)` 启动期填进来
- 字符串 pin 名（`"PA.5"` / `"PD.12"`）—— 调用方写的是字面字符串，看不到 port 索引或寄存器地址。换芯片只改 `pin_board.c` 的解码

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
│   │   ├── platform_pin.h / .c              # PIN device framework (字符串名 + ops 分发)
│   │   ├── platform_assert.h / .c           # platform_assert + 默认 handler
│   │   ├── platform_module_export.h / .c    # 8 级 INIT_xxx_EXPORT 机制 (ARMCC/IAR/GCC + MOCK)
│   │   └── arch/board/
│   │       └── pin_board.c                  # STM32 pin 子类 (依赖 HAL)
│   ├── drivers/
│   │   └── led/
│   │       ├── led_base.h / .c              # 父类接口 + dispatch
│   │       └── led_gpio.h / .c              # GPIO LED 子类
│   └── environment_cfg/
│       └── led_cfg.c                        # 4 颗 LED 实例 + INIT_ENV_EXPORT
└── mock/                                    # PC 模拟模式
    ├── main_pc.c                            # PC 主程序
    └── pin_board_pc.c                       # PC pin 子类 (printf 模拟)
```

## 8 级初始化机制

`platform_module_export.h` 提供 8 个优先级宏：

```c
INIT_BOARD_EXPORT(fn)         /* Level 1: 板级初始化 (pin / clock / reset) */
INIT_PREV_EXPORT(fn)          /* Level 2: 组件预初始化 (heap / sys) */
INIT_DEVICE_EXPORT(fn)        /* Level 3: 设备初始化 */
INIT_COMPONENT_EXPORT(fn)     /* Level 4: 组件初始化 (中间件) */
INIT_ENV_EXPORT(fn)           /* Level 5: 环境配置 (LED / 外设实例装配) */
INIT_APP_EXPORT(fn)           /* Level 6: 应用初始化 */
INIT_SYSTEM_READY_EXPORT(fn)  /* Level 7: 系统就绪 */
UNIT_TEST_EXPORT(fn)          /* Level 8: 单元测试 (跟前 7 级独立) */
```

`platform_module_export_exec()` 启动期顺序跑 1-7 级。第 8 级独立由 `platform_unit_test_exec()` 跑。

跨编译器（ARMCC / IAR / GCC）通过 `__attribute__((section))` + linker 段实现。MOCK 模式下用 `__attribute__((constructor(N)))` 在 main 之前自动跑，应用层调 `platform_module_export_exec()` 是 nop，**调用形态完全一致**。

## 两种 build 模式

### PC 模拟模式（不依赖 STM32 HAL）

直接验证整套 OOP 抽象的业务逻辑。所有 GPIO 操作走 `pin_board_pc.c` 打到 stdout：

```bash
cd industrial/stm32_full
make MOCK=1
./build/firmware-pc.exe       # Windows
./build/firmware-pc           # Linux/Mac
```

预期输出（节选）：

```
    [PC] PD.12 mode -> OUTPUT
    [PC] PD.12 <- LOW
    [PC] PD.13 mode -> OUTPUT
    [PC] PD.13 <- LOW
    [PC] PD.14 mode -> OUTPUT
    [PC] PD.14 <- LOW
    [PC] PD.15 mode -> OUTPUT
    [PC] PD.15 <- LOW
=========================================
  stm32_full PC mock: 4 LED running light
=========================================

--- step 0 ---
    [PC] PD.12 <- LOW
    [PC] PD.13 <- LOW
    [PC] PD.14 <- LOW
    [PC] PD.15 <- LOW
    [PC] PD.12 <- HIGH
...
=========================================
  done (3 rounds)
=========================================
```

3 圈流水灯（12 步）后退出，0 警告 0 错误。

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

预期：板上 4 颗 LED 依次亮灭，1 秒切一颗。

## 跨芯片移植

`app/platform/arch/board/pin_board.c` 是 STM32F4 实现。换 H7/F1/L4/G0：

1. 复制 `pin_board.c` → `pin_board_stm32xx.c`，改 `#include "stm32xxxx_hal.h"`
2. 时钟使能宏 `__HAL_RCC_GPIOx_CLK_ENABLE()` 在不同芯片名字微差，改名字即可
3. 应用层、驱动层（`app/drivers/`）、平台框架（`app/platform/platform_pin.c`、`app/platform/platform_module_export.c`）一字不改

## 跨开发板

板子上 LED 引脚不一样时只改 `app/environment_cfg/led_cfg.c` 的 4 个 pin 名字符串：

- `"PD.12"` → 改成你板子上对应的 `"PA.5"` 之类
- 如果是低电平点亮：`led_gpio_init(..., true)` 的 `true` 改成 `false`

其余文件一字不动。

## 已知工程性细节

工程结构完整、抽象覆盖全。下面几点真机移植时按 CubeMX 生成版补全：

- `linker_stm32f407.ld` 中 `_Min_Heap_Size` / `_Min_Stack_Size` / `.preinit_array` / `.init_array` / `.ARM.exidx`
- `startup_stm32f407xx.s` 的 80+ 中断向量
- `Core/Src/main.c` 中 `SystemClock_Config` 和 `MX_GPIO_Init` 的 HAL 调用细节

这些都是 CubeMX 自动生成的工程性细节，不影响 OOP 抽象的教学价值。
