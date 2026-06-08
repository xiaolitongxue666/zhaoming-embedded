# Project Memory (Compact)

1) **ch15 `pc/` 四层（自上而下）** — ①应用 ②板级 BSP ③设备驱动 ④Platform。层号=离业务/硬件远近，**不等于** `main()` 执行顺序。

2) **Platform 栈底但最先 init** — `platform_init()` 在 `led_board_init()` 之前注册 PWM/I2C ops；跳过则 `_g_ops`/`_g_bus` 为 NULL。

3) **Dispatcher** — `platform/platform_pwm.c`、`platform_i2c.c` 存表并转发；`platform_pwm_register()` 挂后端。同构 ch11 `led_base`+`ops`。

4) **Platform 后端** — PC：`platform_*_pc.c`+`common/platform_pc.c`；真机：`platform/arch/<mcu>/*_board.c`。换 MCU 只改后端。

5) **Board(L2) vs Platform(L4)** — Board 管 pin/通道/地址，仅开机 init；运行时 `led_on` 为 App→Driver→Platform，不经过 Board。

6) **两套四层视角** — pc 教学 Board 独立；换 MCU 表为 App→drivers→platform 接口→arch。Board 移植时通常不改。

7) **GPIO 无 dispatcher（PC）** — `platform_gpio_write` 在 `common/platform_pc.c` 直接实现；PWM/I2C 已 ops+register。

8) **ch15 文档** — `oop-in-c/code/15-platform/pc/README.md`（分层/dispatcher/init 链）；`15-platform/README.md`（换 MCU/arch）。

9) **ch05 GPIO 多实例** — PC 用 `g_gpioa_regs` 模拟 MMIO；`.h` 仅 `extern`+`GPIOA` 宏，定义在 `hal_gpio.c`（ODR）。见 `05-hal-mapping/pc/README.md`。

10) **ch16 路径** — `oop-in-c/code/16-linux-style/pc/`；五层 main→leds_gpio→gpiolib→gpio_chip→vendor。

11) **ch16 dispatch** — `gpiod_set_value` → `desc->gc->set(...)`；`gpio_chip` 五函数指针=接口继承。

12) **ch15 vs ch16** — ch15 单 `_g_ops` 后端；ch16 多 `gpio_chip` 实例，贴近 Linux 多控制器。

13) **N×M→N+M** — gpiolib 中间层；LED 语义(leds_gpio)与 GPIO dispatch(gpiolib) 分离。

14) **vendor_a/b** — DR_REG 片内(base=0,32)；BSRR 外扩(base=32,16)。

15) **构建** — 各章 `pc/`：`make && ./demo`；gcc -Wall -Wextra -std=c99。

16) **2026-06-08 文档/注释** — 新增 `15-platform/pc/README.md`；更新 ch15/ch05 README 与 platform_init/dispatcher/板级/gpio 注释。

17) **FAQ：Platform 最后却先 init** — 层号=依赖方向；init=先通 Platform 管线再配 Board。

18) **FAQ：dispatcher 代码位置** — `platform_pwm.c`/`platform_i2c.c`；注册链 `platform_init`→`platform_pc_*_init`→register。

19) **FAQ：芯片隔离层** — 在 Platform 内 `arch/<mcu>/` 或 PC 后端，下接 HAL，无项目第五层。

20) **FAQ：extern/define 分文件** — 避免 multiple definition；HAL 模块数据归位在 `.c`。
