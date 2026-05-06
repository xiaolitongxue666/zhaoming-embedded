# 附录 C · Linux 用户态完整工程：跑通全书所有抽象

参考代码：[`industrial/linux_full/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/industrial/linux_full/)。

跟附录 B 严格对称，**唯一变化的是 platform 子类**：STM32 用 HAL，Linux 用 libgpiod / sysfs。应用层、驱动层、平台框架四组文件（`led_base.[hc]` / `led_gpio.[hc]` / `platform_pin.[hc]` / `platform_module_export.[hc]`）跟 stm32_full 字节级一致。

读完这一附录你能切身感受到"换平台不改应用"的威力：同一份 OOP 抽象骨架，下面换上 libgpiod 这一片地基，上层一行不动跑在树莓派 4B 上。

## C.1 工程概览

目标硬件：**树莓派 4B**（ARM64 Cortex-A72，4 核 1.5 GHz，4 GB RAM，跑 Raspberry Pi OS / Ubuntu）。这是国内最容易拿到的 ARM SBC，淘宝几百块。

不同 SBC 跑这个工程都是 5% 改动量。香橙派、飞腾派、瑞芯微 RK3588、全志 H6 / H616 等任何一颗，照着 `app/platform/arch/board/pin_libgpiod.c` 改 `GPIO_CHIP_DEV` 一行就能跑。

工程功能：板上 4 颗 LED 流水灯（GPIO17 / GPIO27 / GPIO22 / GPIO23 BCM 编号，对应树莓派 4B 物理引脚 11 / 13 / 15 / 16）。硬件接法：每颗 LED 长脚串 220Ω 限流电阻接 GPIO 引脚，短脚接 GND。

跑通全书所有 OOP 抽象，对应章节同附录 B 的 B.1 节。

## C.2 工程结构

```
industrial/linux_full/
├── README.md
├── Makefile                                 # 三模 build (libgpiod / sysfs / MOCK)
├── src/
│   └── main.c                               # 真机主程序 (流水灯, signal-aware)
├── app/                                     # 应用层 (跟 stm32_full 同源)
│   ├── project_config.h                     # 跨编译器宏 + 开关
│   ├── platform/
│   │   ├── platform_def.h                   # platform_err_t + container_of
│   │   ├── platform_pin.h / .c              # PIN 框架 (字符串名 + ops 分发)
│   │   ├── platform_assert.h / .c
│   │   ├── platform_module_export.h / .c    # 8 级 INIT_xxx_EXPORT (Linux 用 ctor)
│   │   └── arch/board/
│   │       ├── pin_libgpiod.c               # libgpiod 子类 (默认)
│   │       └── pin_sysfs.c                  # sysfs 子类 (退路)
│   ├── drivers/
│   │   └── led/
│   │       ├── led_base.h / .c              # 父类接口 (跟 stm32_full 字节级一致)
│   │       └── led_gpio.h / .c              # GPIO LED 子类 (跟 stm32_full 字节级一致)
│   └── environment_cfg/
│       └── led_cfg.c                        # 4 颗 LED 实例 + INIT_ENV_EXPORT
└── mock/                                    # PC 模拟模式 (跟 stm32_full 等价)
    ├── main_pc.c
    └── pin_board_pc.c
```

跟 stm32_full 做 diff，以下文件**字节级一致**（同一份代码两个工程共用）：

- `app/project_config.h`
- `app/platform/platform_def.h`
- `app/platform/platform_pin.[hc]`
- `app/platform/platform_assert.[hc]`
- `app/platform/platform_module_export.[hc]`
- `app/drivers/led/led_base.[hc]`
- `app/drivers/led/led_gpio.[hc]`

差异只在 `app/platform/arch/board/`（libgpiod / sysfs vs HAL）、`app/environment_cfg/led_cfg.c`（BCM 编号 vs PD 编号）、`src/main.c`（sleep + signal vs busy-wait + NVIC）、`Makefile`（三模 vs 双模）。

## C.3 关键文件

### app/platform/arch/board/pin_libgpiod.c（libgpiod 子类·默认）

基于 libgpiod 1.x API。Linux 内核 4.8 后官方推荐，sysfs 接口已经标记为 deprecated。

```c
#include <gpiod.h>

#define GPIO_CHIP_DEV  "gpiochip0"   /* 树莓派默认控制器 */
#define MAX_LINES      64

struct line_slot {
    int                pin;
    int                mode;
    struct gpiod_line *line;
};

static struct gpiod_chip *_g_chip;
static struct line_slot   _g_slots[MAX_LINES];
static int                _g_slot_count;

static int32_t _libgpiod_pin_get(const char *name)
{
    /* "GPIO17" / "GPIO27" -> BCM 编号 */
    if ((name[0] != 'G') || (name[1] != 'P') ||
        (name[2] != 'I') || (name[3] != 'O'))
        return PLATFORM_EINVAL;
    int n = atoi(name + 4);
    if ((n < 0) || (n > 53))
        return PLATFORM_EINVAL;
    return n;
}

static struct line_slot *_acquire_slot(int pin, int mode)
{
    /* 第一次拿这个 pin 时打开 chip + request_output, 之后缓存到 slot 表 */
    /* ... */
}

static void _libgpiod_pin_write(int32_t pin, int32_t value)
{
    struct line_slot *slot = _find_slot(pin);
    if (slot)
        gpiod_line_set_value(slot->line, value ? 1 : 0);
}

static const platform_pin_ops_t _libgpiod_pin_ops = {
    .pin_mode  = _libgpiod_pin_mode,
    .pin_write = _libgpiod_pin_write,
    .pin_read  = _libgpiod_pin_read,
    .pin_get   = _libgpiod_pin_get,
};

static void _platform_hw_pin_init(void)
{
    platform_pin_register(&_libgpiod_pin_ops);
}
INIT_BOARD_EXPORT(_platform_hw_pin_init);
```

跟 stm32_full 的 `pin_board.c` 在结构上完全一致：4 个 ops 函数、`platform_pin_register` 注册、`INIT_BOARD_EXPORT` 启动期自动调。

### app/platform/arch/board/pin_sysfs.c（sysfs 子类·退路）

```c
static void _sysfs_pin_write(int32_t pin, int32_t value)
{
    char path[64];
    snprintf(path, sizeof(path),
             "/sys/class/gpio/gpio%d/value", (int)pin);
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        write(fd, value ? "1" : "0", 1);
        close(fd);
    }
}
```

sysfs 接口在内核 4.8 后被官方标记为 deprecated，新工程优先用 libgpiod。这里保留 sysfs 是为了演示"切 backend 时上层一字不改"，同一个 `app/drivers/led/led_gpio.c` 调 `platform_pin_write(self->pin_num, ...)`，下面是 sysfs 写文件还是 libgpiod 调 `gpiod_line_set_value`，driver 一字不知。

### app/environment_cfg/led_cfg.c（BCM 引脚配置）

```c
static led_gpio_t _led_green_inst;
static led_gpio_t _led_orange_inst;
static led_gpio_t _led_red_inst;
static led_gpio_t _led_blue_inst;

led_base_t *led_green   = NULL;
led_base_t *led_orange  = NULL;
led_base_t *led_red     = NULL;
led_base_t *led_blue    = NULL;

static void _led_cfg(void)
{
    led_gpio_init(&_led_green_inst,  "GPIO17", true);
    led_gpio_init(&_led_orange_inst, "GPIO27", true);
    led_gpio_init(&_led_red_inst,    "GPIO22", true);
    led_gpio_init(&_led_blue_inst,   "GPIO23", true);

    led_green  = (led_base_t *)&_led_green_inst;
    led_orange = (led_base_t *)&_led_orange_inst;
    led_red    = (led_base_t *)&_led_red_inst;
    led_blue   = (led_base_t *)&_led_blue_inst;
}
INIT_ENV_EXPORT(_led_cfg);
```

跟 stm32_full 的 `led_cfg.c` 区别只在 4 个 pin 字符串：`"PD.12"` / `"PD.13"` / `"PD.14"` / `"PD.15"` 换成 BCM 风格的 `"GPIO17"` / `"GPIO27"` / `"GPIO22"` / `"GPIO23"`。其他一字不动。

### src/main.c（Linux 用户态主程序）

```c
#include <signal.h>
#include <unistd.h>
#include "led/led_base.h"
#include "platform/platform_module_export.h"

extern led_base_t *led_green;
extern led_base_t *led_orange;
extern led_base_t *led_red;
extern led_base_t *led_blue;

static volatile int _g_running = 1;

static void _on_sigint(int sig) { (void)sig; _g_running = 0; }

int main(void)
{
    led_base_t *all[4];
    uint32_t cur = 0;

    signal(SIGINT,  _on_sigint);
    signal(SIGTERM, _on_sigint);

    platform_module_export_exec();   /* nop, ctor 已跑完 */

    all[0] = led_green;
    all[1] = led_orange;
    all[2] = led_red;
    all[3] = led_blue;

    while (_g_running) {
        for (uint32_t i = 0; i < 4; i++)
            led_off(all[i]);
        led_on(all[cur]);
        cur = (cur + 1) % 4;
        sleep(1);
    }

    /* 清退: 关掉所有 LED */
    for (uint32_t i = 0; i < 4; i++)
        led_off(all[i]);
    return 0;
}
```

跟 stm32_full 的 main.c 区别：

- `signal()` 注册 SIGINT / SIGTERM，Ctrl+C 优雅退出（清退所有 LED）
- `sleep(1)` 替代 busy-wait
- 不需要 `HAL_Init` / `SystemClock_Config`，glibc 启动期已经搞定

### Linux 用户态怎么实现 8 级 initcall

Linux 用户态有现成的机制：GCC `__attribute__((constructor(N)))`。所有 INIT_xxx_EXPORT 注册项被编译为 ctor 函数，ELF loader 在 main 之前自动按 priority 顺序跑完。

跟 stm32_full 的 linker section 路径相比，Linux 用户态简化得多，不需要写 linker 脚本里的 `__moduleExportN_start / _end` 边界符号。

`platform_module_export.h` 用 `#ifdef MOCK_PC` 切换两条路径：

- 真机 STM32 / IAR / ARMCC：linker section
- Linux 用户态 / PC mock：GCC ctor

应用层调 `platform_module_export_exec()` 在两条路径下行为完全一致：真机走遍 7 级 section，Linux 是 nop（ctor 在 main 之前已经跑完）。**调用形态一字不变**。

## C.4 编译运行步骤

### PC 模拟模式（不依赖 GPIO 硬件）

```bash
cd industrial/linux_full
make MOCK=1
./build/firmware-pc
```

预期输出（节选）：

```
=========================================
  linux_full PC mock: 4 LED running light
=========================================

--- step 0 ---
    [PC] GPIO17 <- LOW
    [PC] GPIO27 <- LOW
    [PC] GPIO22 <- LOW
    [PC] GPIO23 <- LOW
    [PC] GPIO17 <- HIGH
...
=========================================
  done (3 rounds)
=========================================
```

### libgpiod 模式（真机·树莓派 4B 推荐）

```bash
sudo apt install libgpiod-dev    # Debian/Ubuntu
# 或: sudo dnf install libgpiod-devel  # Fedora/RHEL

make
sudo ./build/firmware
```

`/dev/gpiochip0` 是树莓派 4B 默认 GPIO 控制器。其他 SBC 改 `app/platform/arch/board/pin_libgpiod.c` 的 `GPIO_CHIP_DEV` 字符串：

| 板子 | 控制器名 |
|---|---|
| 树莓派 4B / 5 | gpiochip0 |
| 香橙派 5 | gpiochip1 (sunxi-gpio) |
| 飞腾派 D2000 | gpiochip0 |
| RK3588 SBC | gpiochip0 / gpiochip1（看具体板） |

`gpiodetect` 命令可以列出板上所有 controller，挑写在板子上 LED 实际接的那一个。

### sysfs 模式（退路）

```bash
make BACKEND=sysfs
sudo ./build/firmware
```

sysfs 在内核 4.8 后被标记为 deprecated，但仍然能用。优先选 libgpiod。这里保留 sysfs 是为了演示"切 backend 时上层一字不改"。

### 接线

树莓派 4B 默认 GPIO 引脚映射：

| 颜色 | BCM 编号 | 物理引脚 |
|---|---|---|
| 绿 | GPIO17 | 11 |
| 橙 | GPIO27 | 13 |
| 红 | GPIO22 | 15 |
| 蓝 | GPIO23 | 16 |

每颗 LED 长脚（正极）串 220Ω 限流电阻接 BCM 引脚，短脚（负极）接 GND（物理引脚 6 / 9 / 14 等任一 GND 都行）。

如果你的板子接法不一样，改 `app/environment_cfg/led_cfg.c` 4 个字符串：

```c
led_gpio_init(&_led_green_inst,  "GPIO17", true);   /* 改成你板上的 BCM 编号 */
```

`true` / `false` 控制点亮电平：高电平点亮 → `true`，低电平点亮 → `false`。

## C.5 这个工程跑通了什么

按全书 18 章过一遍：

| 章节 | 抽象 | 在这个工程里的位置 |
|---|---|---|
| ch01 | struct + me | 所有 `led_xxx` 函数第一参数 `led_base_t *me` |
| ch02 | 信息隐藏 | `led_gpio_t` 字段定义在 `led_gpio.h`，应用层只看到 `led_base_t *` |
| ch03 | class 化 | `led_gpio_init / led_on / led_off` 命名前缀统一 |
| ch04 | 数据归位 | 静态实例 `_led_green_inst`（数量固定的全局对象选静态实例） |
| ch05 | HAL 漫游 | `pin_libgpiod.c` 里 `gpiod_line_set_value` / `pin_sysfs.c` 里 sysfs 文件接口 |
| ch06 | 继承痛点 | `led_gpio_t` 把 `led_base_t` 嵌在第一字段 |
| ch07 | 函数指针 | `led_base_ops_t` 里的函数指针 |
| ch08 | 函数指针传参 | `platform_pin_register(&_libgpiod_pin_ops)` 把整张 ops 表当参数传 |
| ch09 | ops 表雏形 | `platform_pin_ops_t` |
| ch10 | ops 放进对象 | `led_base_t.ops` 字段 |
| ch11 | 多态完整 | `led_on(led_blue)` 跟 `led_on(led_green)` 调用一字不差，底下走不同实现 |
| ch12 | 向上转型 | `led_green = (led_base_t *)&_led_green_inst;` |
| ch13 | container_of | `led_base.c` 三层 `platform_assert` 校验后 dispatch |
| ch14 | 纯虚 | `led_base_ops_t` 里 `led_on / led_off` 是 pure virtual |
| ch15 | platform 抽象到底 | 同一份 driver 切 libgpiod / sysfs / mock 三种 backend，应用层一字不改 |
| ch16 | Linux 风格 | `container_of`、`platform_pin_register`、`INIT_xxx_EXPORT` 全是 Linux 内核风 |
| ch17 | 链接初始化 | `INIT_xxx_EXPORT` 通过 GCC ctor 自动跑 |

## C.6 跟 stm32_full 的差异

读完附录 B 再读这一附录，你应该能看出整本书的核心 takeaway：**换平台只动 platform 子类，应用层和驱动层一字不动**。

| 维度 | stm32_full | linux_full |
|---|---|---|
| 启动 | startup_*.s + Reset_Handler + linker section initcall | ELF loader + GCC ctor initcall |
| pin 子类 | HAL_GPIO_xxx | libgpiod / sysfs |
| pin 字符串 | "PA.5" / "PD.12"（port-pin 编码） | "GPIO17" / "GPIO27"（BCM 编号） |
| 主循环延时 | busy-wait `for (volatile i ...)` | `sleep(1)` |
| 信号处理 | NVIC 中断 | SIGINT / SIGTERM `signal()` |
| Make 目标 | `make` + `make flash` | `make` + `sudo ./build/firmware` |

应用层、驱动层、平台框架（4 + 8 = 12 个文件）完全字节级一致。这就是这本书把"换硬件不改应用"做实的证据。

## C.7 进阶练习

如果你跑通了流水灯，下面几个改造可以验证你对抽象的理解：

**练习 1：** 加按钮（GPIO 输入）。新增 `app/drivers/button/`，参照 LED 子类的写法，配 `platform_pin_attach_irq` 处理按下事件（libgpiod 提供 `gpiod_line_event_wait` 等待边沿）。在 main 里把按钮绑定到"按下切换 LED 模式"。

**练习 2：** 把 4 颗 LED 改成 PWM 软件控制，加 `led_pwm.c` 子类，主循环里跑呼吸灯效果。

**练习 3：** 双平台同时维护：把 stm32_full 和 linux_full 共用的 7 个文件抽到 `industrial/common/`，两个工程的 Makefile 都从 `common/` 编。验证你对"应用层 / 驱动层平台无关"的理解。

## C.8 已知工程性细节

工程结构完整、抽象覆盖全，PC 模拟模式 0 警告 0 错误跑通。下面几点真机移植时需要注意：

- **libgpiod 1.x vs 2.x API 差异**：本工程用 1.x（更广泛）。Debian 12 / Ubuntu 22.04 默认装的就是 1.x。如果你的发行版只有 2.x（API 改了 request builder），需要改 `pin_libgpiod.c` 的几个调用。
- **sysfs 不再默认启用**：内核 6.5 之后部分发行版默认编译关闭 sysfs GPIO，只能用 libgpiod。如果 `/sys/class/gpio/` 不存在，就是这个原因。
- **ELF constructor priority** 在不同 glibc 版本上的行为统一，没遇到过实际兼容问题。

## 下一篇

[附录 D · 配套代码索引 + 视频清单 + 编译运行指南](D-配套代码索引.md)
