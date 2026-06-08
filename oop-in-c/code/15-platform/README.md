# ch15 · 换硬件不改应用 · OOP 完整框架

配套书章节：[`book/04-工程威力/15-Platform抽象.md`](../../../book/04-工程威力/15-Platform抽象.md)

## 看点

ch15 把整本书 ch01 - ch14 学过的所有 OOP 武器组装成一份完整 LED 框架，**0 个新概念**。`pc/` 是 PC 教学四层架构 (`make && ./demo` 直接跑)。本目录最有看点的是 `drivers/` + `platform/` + `platform/arch/<mcu>/` 这一组工业级跨 MCU 分层 -- 演示 § 15.11.5 "STM32 vs NXP 换 MCU 不改应用" 的代码兑现点。

## 目录结构

```
15-platform/
├── pc/                              PC 教学版四层架构 (make && ./demo)
├── drivers/                         设备驱动层·跨 MCU 不变
│   └── led/
│       ├── led_base.{h,c}           父类 + ops 表 + 父类统一接口
│       ├── led_gpio.{h,c}           子类一: GPIO LED
│       ├── led_pwm.{h,c}            子类二: PWM LED (调亮度)
│       └── led_i2c.{h,c}            子类三: I2C 扩展芯片 LED
├── platform/                        platform 接口层·跨 MCU 不变
│   ├── platform_pin.{h,c}           ops 表 + register dispatcher
│   ├── platform_pwm.{h,c}
│   ├── platform_i2c.{h,c}
│   └── arch/                        MCU 厂家差异收拢点
│       ├── stm32/                   STM32 HAL 后端 (3 份, 一外设一文件)
│       │   ├── pin_board.c          GPIO 后端 (HAL_GPIO_WritePin)
│       │   ├── pwm_board.c          PWM 后端 (TIM3 + __HAL_TIM_SET_COMPARE)
│       │   └── i2c_board.c          I2C bus 后端 (HAL_I2C_Master_Transmit)
│       └── nxp/                     NXP MCUXpresso SDK 后端 (3 份, 同款拆分)
│           ├── pin_board.c          GPIO 后端 (GPIO_PinWrite)
│           ├── pwm_board.c          PWM 后端 (PWM1 + PWM_UpdatePwmDutycycle)
│           └── i2c_board.c          I2C bus 后端 (LPI2C_MasterTransferBlocking)
└── linux-driver/
    └── userspace/                   Linux 用户态 (直接 libgpiod / sysfs / i2c-dev)
```

## 四层分工·一图看清

| 文件 | 跨 MCU 是否变化 |
| --- | --- |
| `main.c` / `app.c` (应用层) | 不变 |
| `drivers/led/*` 4 对 .h/.c (设备驱动层) | 不变 |
| `platform/platform_*.h` `platform/platform_*.c` (platform 接口层) | 不变 |
| `platform/arch/<mcu>/{pin,pwm,i2c}_board.c` (platform 实现层 = 3 份) | **唯一变化点** |

应用层 + drivers/led + platform 接口层全部跨 MCU 一字不动。**唯一变化点是 `platform/arch/<mcu>/` 下的 6 份 board 文件之一** (一家 MCU 三份, 一外设一文件)。换 MCU 改这 3 份, 应用层一字不动。这就是 platform 层抽象的真正威力。

一外设一文件的好处: 加一种新外设 (uart / spi) 只新增 `uart_board.c` / `spi_board.c` 一份, 已有三份字节不动; review 时也能 `git log pwm_board.c` 直接拿 PWM 这一路独立的演化历史。

## STM32 vs NXP 三外设对照

把 `platform/arch/stm32/{pin,pwm,i2c}_board.c` 和 `platform/arch/nxp/{pin,pwm,i2c}_board.c` 六份并排打开, 每外设一对:

| 外设 | `stm32/xxx_board.c` 调用 | `nxp/xxx_board.c` 调用 |
| --- | --- | --- |
| GPIO | `HAL_GPIO_WritePin` | `GPIO_PinWrite` |
| PWM | `__HAL_TIM_SET_COMPARE` | `PWM_UpdatePwmDutycycle` |
| I2C | `HAL_I2C_Master_Transmit` | `LPI2C_MasterTransferBlocking` |
| 时钟使能 | `__HAL_RCC_GPIOA_CLK_ENABLE` | MCUXpresso `BOARD_BootClockRUN` |
| 启动入口 | `platform_hw_{pin,pwm,i2c}_init()` | 同左 (签名一字不差) |

GPIO 一段并排:

```c
/* arch/stm32/pin_board.c */
static void _stm32_pin_write(int32_t pin, int32_t value)
{
    GPIO_TypeDef *port = _gpio_table[PIN_PORT(pin)];   /* GPIOA..GPIOI */
    HAL_GPIO_WritePin(port, PIN_MASK(pin),
                      value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* arch/nxp/pin_board.c */
static void _nxp_pin_write(int32_t pin, int32_t value)
{
    GPIO_Type *port = _gpio_table[PIN_PORT(pin)];      /* GPIO1..GPIO5 */
    GPIO_PinWrite(port, PIN_OFFSET(pin), value ? 1U : 0U);
}
```

类型 (`GPIO_TypeDef *` vs `GPIO_Type *`)、寄存器 API 名字 (`HAL_GPIO_WritePin` vs `GPIO_PinWrite`)、PWM 引擎 (TIM3 vs eFlexPWM PWM1)、I2C 引擎 (HAL `I2C` vs LPI2C) 是两家厂家的差异。这种厂家差异如果漏到 `drivers/led/led_gpio.c` / `led_pwm.c` / `led_i2c.c`, 换 MCU 要改三个子类 + 板级 + 应用层 -- ch15 的金句"换硬件不改应用"就破了。

`platform/arch/<mcu>/{pin,pwm,i2c}_board.c` 这一层就是为了把这种厂家差异收拢到一组三份文件里。板级启动序列也是一字对照: `platform_init` 里依次调 `platform_hw_pin_init() -> platform_hw_pwm_init() -> platform_hw_i2c_init()` 三行, STM32 / NXP 两家板级代码这三行完全一样, 改的只是链接的 `arch/stm32/` 还是 `arch/nxp/` 子目录里那 3 份后端。LED 模块自己的硬件参数走 `led_board_init`, 跟 platform 注册分两层职责。

## `pc/` 是什么·跑什么

PC 教学版四层架构, `make && ./demo` 直接在 PC 上跑出 GPIO + PWM + I2C 三盏灯混搭的开机自检 / 报警闪烁 / 状态指示流程。三盏灯过同一份父类接口 `led_on / led_off`, 落到不同子类实现, 最终调到 `platform_gpio_write` 等封装函数, 由 `../../common/platform_pc.c` 用 `printf` 模拟。

**分层、dispatcher、启动顺序 vs 层号、Platform 层意义** 的详细说明见 [`pc/README.md`](pc/README.md)。

```
cd pc
make
./demo
```

输出: 开机自检 -> 报警闪烁 -> 状态指示。三盏灯 (GPIO + PWM + I2C 混搭) 经过同一份 `led_on / led_off` 父类接口, 分发到不同子类。

### pc/ 四层 (教学视角) vs 换 MCU 四层

| pc/ 教学四层 (自上而下) | 换 MCU 视角四层 | 说明 |
| --- | --- | --- |
| ① `main.c` / `app.c` 应用 | 应用 | 业务不变 |
| ② `led_board_init.c` 板级 BSP | (与应用一样通常不改) | 换 PCB 接线改这里 |
| ③ `led_base.*` / `led_gpio.*` … 驱动 | `drivers/led/*` | 跨 MCU 不变 |
| ④ `platform_init` + dispatcher + `platform_*_pc.c` | platform 接口 + `arch/<mcu>/` | 换 MCU 改后端 |

层号按 **谁调用谁、离硬件远近** 编号, **不是** `main()` 执行顺序. Platform 虽是最底层, 但 `platform_init()` 必须 **最先** 跑 (给 dispatcher 注册 ops), 之后才是 `led_board_init()` 和应用代码.

### Dispatcher (分发器)

`platform/platform_pwm.c` / `platform_i2c.c` 维护 `_g_ops` / `_g_bus`: 启动期 `platform_pwm_register()` 把 PC 或 STM32 后端的 ops 表挂上去, 运行时 `platform_pwm_enable()` 等固定 API 内部 **转发** 到已注册实现. 与 ch11 `led_base` + `me->ops` 同构, 只是 ops 对象从 LED 子类换成了 MCU 后端. PC 端注册链: `main` → `platform_init()` → `platform_pc_pwm_init()` → `platform_pwm_register(&pc_pwm_ops)`.

### 为何要 Platform 层 · 芯片隔离在哪

**意义**: 把 STM32 / NXP / PC 的厂家差异关进 Platform 后端, `drivers/led/*` 只调 `platform_pwm_*` / `platform_i2c_*`, 换 MCU 时应用 + driver + dispatcher 源码不动.

**芯片隔离层**: 就在 Platform **内部下半段** —— 真机 `platform/arch/<mcu>/{pin,pwm,i2c}_board.c`, PC 上对应 `pc/platform_pwm_pc.c` / `platform_i2c_pc.c` / `common/platform_pc.c`. 其下是厂家 HAL/CMSIS, 本项目一般 **不再** 自建第五层.

## `drivers/` + `platform/` + `platform/arch/<mcu>/` 是什么

§ 15.11.5 "STM32 vs NXP" 的代码兑现层。同一套四层架构落到两家厂家的真机上。

`drivers/led/*.c` 子类只调 `platform_pin_xxx / platform_pwm_xxx / platform_i2c_xxx` ops 表层接口, 永远不直接 #include 厂家 SDK 头. `platform/*.c` dispatcher 维护 `static const struct platform_xxx_ops *_g_ops`, 启动期由 `platform/arch/<mcu>/{pin,pwm,i2c}_board.c` 三份后端的 `platform_hw_pin_init()` / `platform_hw_pwm_init()` / `platform_hw_i2c_init()` 各调一次 register 函数填进来. 上层调封装函数时框架内部 dispatch 到对应 MCU 的实现.

PIN 编码: 高 4 位 port (字母 A-Z 偏移), 低 4 位 num. 接口签名永远只有一个 `pin` 参数 -- 换 MCU (port 数量、命名都变) 时, 接口不动, 只要 `platform/arch/<mcu>/pin_board.c` 的解码表跟着换。

这一份代码不在本目录直接 `make` 跑 -- 真机移植参考代码, 需要对应工程的 STM32CubeMX / MCUXpresso 工程上下文。读代码看分层结构即可, 真要跑就把这一档拷进 STM32CubeIDE / MCUXpresso 工程对应位置。

## `linux-driver/userspace/` 是什么

Linux 用户态 LED 框架。**这一档目录没有 platform 抽象层**。

`led_gpio.c` 直接调 libgpiod, `led_pwm.c` 直接 open / write sysfs PWM 节点, `led_i2c.c` 直接 open `/dev/i2c-N` + ioctl。Linux 内核 driver model + chardev + sysfs 已经把硬件抽得干干净净, 应用层再套一层 `platform_gpio_write -> gpiod_line_set_value` 就是过度封装。

ch15 § 15.11 / § 15.15 / § 15.16 反复讲过这件事, 这一档目录是代码兑现层。父类 ops 表保留 (OOP 抽象任何平台都该有)。

LED 这种通用外设 Linux 内核已经有 `drivers/leds/leds-gpio.c` (标准 driver model 写好的内核驱动), 应用层不该再抽自家内核驱动 -- 自抽一份反而误导读者绕开内核标准接口。完整论述见 ch16 § 16.13 / 附录 C, 真要看内核 LED 驱动长什么样, 直接 grep 内核源 `drivers/leds/leds-gpio.c`.

## 跑

```
cd pc
make
./demo
```

`drivers/` + `platform/` 这两档目录是真机移植参考代码, 不在本目录直接 `make` 跑。`linux-driver/userspace/` 真机跑见子目录 README。
