# 附录 C · Linux 用户态完整工程：内核已做完 platform·应用层直接用

参考代码：[`industrial/linux_full/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/industrial/linux_full/)。

附录 B 给的是 STM32 裸机的完整工程，里面有完整的 `app/platform/` 抽象层。这一份附录给的是 Linux 用户态的完整工程，**没有 platform 抽象层**。同一组 `struct led_base + 多子类 + struct led_base *` 的 OOP 抽象，下面换成在 Linux 上直接调 libgpiod / sysfs PWM / i2c-dev，应用层、`led_base.[hc]` 接口、4 颗 LED 的句柄声明，跟附录 B 字节级一致。

读完这一附录，你能切身体会到这本书工程判断力教学的核心 takeaway：**抽不抽 platform 层不是教条，要看你跑在哪个环境**。STM32 裸机上必须自抽，Linux 用户态上禁自抽。判断依据是"内核有没有把 platform 抽象做完"。

### 附录 C 在全书的位置

附录 C 跟附录 B 配对，是这本书"内核做完别再抽"的实战兑现：

- **ch15 § 15.17**（i2c 二层 + Linux i2c 子系统对照）：把 Linux 内核 i2c subsystem 的 adapter / 设备两层 ops 跟自抽 platform 层并排放，看清"内核已经把 platform 层做完"长什么样
- **ch16 § 16.13**（Zephyr / RT-Thread 也是同款）：带 device subsystem 的 RTOS 都禁自抽，不只是 Linux
- **ch16 § 16.14**（应用层 vs 内核层判断三步）：在 Linux 这一档环境上，新硬件驱动该写在应用层还是内核层。99% 的 GPIO / I²C / SPI / 温度传感器走应用层就够，附录 C 的 4 颗 LED 是这条判断的具体兑现
- **附录 C 工程**（`industrial/linux_full/`）：用户态调 libgpiod / sysfs / i2c-dev，零中间层。LED 这种通用外设连"自家内核驱动"都不写，要看真实工业级原型直接读内核源 `drivers/leds/leds-gpio.c`（上千种板子用过的标准版本）

附录 B 是"裸机必须自抽 platform"的实战，附录 C 是"内核做完别再抽"的实战。两份工程合起来，是这本书工程判断力教学的最高制高点：看到一份 Linux 应用层代码自抽 platform_pin，第一反应是"这一层是不是多余"；看到一份裸机 STM32 代码不抽 platform 层、应用直接调 HAL_GPIO_WritePin，第一反应是"换芯片就崩"。这两种反应分得清，工程判断力就立住了。

## C.0 为什么这一份工程没有 platform 层

附录 B 的 stm32_full 工程有完整的 `app/platform/` 抽象层（platform_pin / platform_pwm / platform_i2c + 子类 ops 表 + register 机制 + 8 级 initcall）。这一份 Linux 用户态工程没有。原因不是工程偷懒，是 **Linux 内核已经把 platform 抽象做完了**，应用层再套一层就是过度封装。

具体看：

| 抽象层级 | stm32_full（裸机） | linux_full（用户态） |
|---|---|---|
| 应用层 | 调 `led_on(handle)` | 调 `led_on(handle)` |
| OOP base + 子类 | `struct led_base` + `led_gpio` / `led_pwm` / `led_i2c` | 同款 |
| platform 层 ops 表 | **必须自抽**（HAL 不是设备模型） | **禁自抽**（内核已做完） |
| 板级实现 | `pin_board.c`（调 HAL）/ `pwm_board.c` / `i2c_board.c` | **不存在**（libgpiod 就是内核暴露给用户的 platform 接口） |
| initcall 机制 | linker section + 启动期 `module_export_exec()` | `main` 里手动调 `environment_init()` |
| 硬件层 | STM32 寄存器 | Linux 内核 driver model |

linux_full 的 `led_gpio.c` 直接 `#include <gpiod.h>` 调 libgpiod，`led_pwm.c` 直接写 `/sys/class/pwm/`，`led_i2c.c` 直接 `open("/dev/i2c-N") + ioctl(I2C_SLAVE)`。这些 Linux 用户态接口本身就是内核完整 driver model 暴露给用户态的 OOP 接口：内核里 `struct gpio_chip`、`struct pwm_chip`、`struct i2c_adapter` 的 ops 表已经把所有平台差异收拾干净了。

读者要带走的工程判断力：

- **裸机 / 简单 RTOS（FreeRTOS 单纯 kernel）**：必须自抽 platform 层。HAL 只是寄存器封装，不是设备模型，应用层不抽就硬编死在某颗 MCU 上
- **带 device subsystem 的 RTOS（Zephyr / RT-Thread / NuttX）**：内核已经抽好了 device tree + driver model，应用层别再抽，直接用 `device_get_binding` / `rt_device_find` 就行
- **Linux 用户态**：内核做完了，直接 libgpiod / i2c-dev / sysfs PWM
- **Linux 内核态驱动**：用内核 driver model（`platform_driver` / `i2c_driver` / `spi_driver`），不要在内核驱动里再造 platform 层

OOP 抽象（`struct led_base + 多子类多态 + 设备句柄统一导出`）在 Linux 用户态仍然有用，它解决的是"应用层不知道下层硬件细节"，让 main.c 看到的还是 4 个 `struct led_base *`，而不是 1 个 libgpiod handle + 1 组 sysfs fd + 1 个 i2c-dev fd 的混合堆。但 platform 抽象层只在没有现成设备模型的环境才有价值，**在 Linux 上是反工程**。

附录 B 给"必须自抽 platform"的实战，附录 C 给"内核做完别再抽"的实战，两者合起来是这本书工程判断力的最高制高点。

这一份 linux_full 是"内核做完别再抽"的实战, 和附录 B stm32_full "裸机必须自抽"形成完整对照. 两个工程合起来, 读者掌握的是工程判断力, 不是无脑套抽象. 同一份 `app/drivers/led/led_base.[hc]` + 同一组 4 颗 LED 句柄声明, 在 STM32 那一边走自抽 platform 层下沉到寄存器, 在 Linux 这一边直接走 libgpiod / sysfs PWM / i2c-dev 调内核. 应用层一字不动, 子类层差别就锁在子类 .c 一个文件里.

最后再呼应 ch15 § 15.16 + ch16 § 16.14 给真实工程的建议: MCU 项目优先 Zephyr / RT-Thread, 内核已经做完 device + driver 框架, 你 ch15 / ch16 学的所有 ops 表 + 多子类多态它们都内置了. 别走 "FreeRTOS + HAL + 自抽 platform 层" 这种重复造轮子的路径 -- 维护成本你扛不住, 也比不过 Linux Foundation / RT-Thread 社区上千人维护的版本. MPU / SoC 直接用 Linux. 全平台都不要自己抽 platform 层, 这才是工业级.

## C.1 工程概览

工程目标：**在 Linux 上演示 OOP 抽象（多子类多态 + 设备句柄统一导出），不带 platform 抽象层**。

目标硬件：树莓派 4B / 5、香橙派、飞腾派、瑞芯微 RK3588 任何一颗能跑 Linux 的 SBC。淘宝几百块。

工程功能：板上 4 颗 LED 演示 GPIO + PWM + I2C 三种子类混搭。

| 句柄 | 子类 | 资源 |
|---|---|---|
| `led_status` | `struct led_gpio` | gpiochip0 line 17（BCM 17），高电平点亮 |
| `led_dimmer` | `struct led_pwm`  | pwmchip0/pwm0，1 kHz |
| `led_panel`  | `struct led_i2c`  | /dev/i2c-1，addr 0x3C，reg 0x00 |
| `led_alarm`  | `struct led_gpio` | gpiochip0 line 22（BCM 22），高电平点亮 |

应用层 `#include "environment_cfg/environment_export.h"` 一次拿到 4 个 `struct led_base *` 句柄，看不到底下是 GPIO / PWM / I2C 哪种子类，更看不到 libgpiod / sysfs / i2c-dev 这些 Linux 接口细节。

跟 stm32_full 字节级一致的部分：

- `app/drivers/led/led_base.h`（公开接口 + ops 表 + struct 定义）
- `app/drivers/led/led_base.c`（dispatch 三层校验）
- `app/environment_cfg/environment_export.h`（4 个句柄声明）

跟 stm32_full 不同的部分（**这才是这本书的对比点**）：

- `app/drivers/led/led_gpio.[hc]`：底下从 platform_pin 换成 libgpiod
- `app/drivers/led/led_pwm.[hc]`：从 platform_pwm 换成 sysfs PWM
- `app/drivers/led/led_i2c.[hc]`：从 platform_i2c 换成 i2c-dev
- `app/environment_cfg/led_cfg.c`：从 `INIT_ENV_EXPORT` 换成 `environment_init()` 显式调用
- 整个 `app/platform/` 目录消失
- 整个 `mock/` 目录消失（Linux 工程在 Linux 上跑就是真机，不需要跨 OS mock）

## C.2 工程结构

```
industrial/linux_full/
├── README.md
├── Makefile                                # 单一 build 模式 (依赖 libgpiod-dev)
├── src/
│   └── main.c                              # 显式 environment_init() / exit()
├── app/
│   ├── project_config.h                    # LED_ASSERT_HALT 等小开关
│   ├── include/
│   │   ├── led_errors.h                    # platform_err_t enum (跟 stm32_full 接口对齐)
│   │   └── led_assert.h                    # led_assert 宏
│   ├── drivers/
│   │   └── led/
│   │       ├── led_base.h / .c             # 父类 dispatch + assert handler
│   │       ├── led_gpio.h / .c             # 直接调 libgpiod
│   │       ├── led_pwm.h  / .c             # 直接写 /sys/class/pwm/
│   │       └── led_i2c.h  / .c             # 直接走 /dev/i2c-N + ioctl(I2C_SLAVE)
│   └── environment_cfg/
│       ├── environment_export.h            # 4 个句柄 + environment_init/exit
│       └── led_cfg.c                       # gpiod_chip_open + 3 子类装配
└── syntax_stubs/                           # 不在 Linux / 没装 libgpiod 时的 syntax check 占位
    ├── README.md
    ├── gpiod.h
    ├── linux/i2c-dev.h
    └── sys/ioctl.h
```

跟 stm32_full 做一次目录 diff：

- `app/platform/` 整个目录砍掉了
- `mock/` 整个目录砍掉了
- 多了一个 `app/include/`（错误码 + assert 宏，原来归 `app/platform/`，现在归一个独立的小头文件）
- 多了一个 `syntax_stubs/`（Windows / 没装 libgpiod-dev 时跑 `make check-syntax` 用）

## C.3 关键文件

### app/drivers/led/led_base.h（父类公开接口·跟 stm32_full 字节级一致）

```c
struct led_ops {
    platform_err_t (*on)(struct led_base *me);
    platform_err_t (*off)(struct led_base *me);
    platform_err_t (*set_brightness)(struct led_base *me, uint8_t level);
};

struct led_base {
    const struct led_ops *ops;
    const char           *name;
    bool                  is_on;
};

platform_err_t led_base_init(struct led_base *me, const char *name,
                             const struct led_ops *ops);
platform_err_t led_on(struct led_base *me);
platform_err_t led_off(struct led_base *me);
platform_err_t led_set_brightness(struct led_base *me, uint8_t level);
```

跟附录 B 完全一致。`on` / `off` 纯虚（子类必填，父类 dispatch 时 `led_assert`），`set_brightness` 选填（子类不填走父类默认 no-op）。`led_errors.h` 里定义 `platform_err_t` enum（4 个值：`EOK / EINVAL / EIO / ENOMEM`），这个名字保留是为了让两个工程的 led_base 头文件接口一致；底下错误码语义对齐 POSIX errno，因为子类直接调 libgpiod / sysfs / i2c-dev，错误来源就是这一组。

### app/drivers/led/led_gpio.c（直接调 libgpiod·**这一份是关键对比**）

```c
#include <gpiod.h>

struct led_gpio {
    struct led_base    base;
    struct gpiod_line *line;
    bool               active_high;
};

platform_err_t led_gpio_init(struct led_gpio *me, const char *name,
                             struct gpiod_chip *chip,
                             unsigned int line_offset, bool active_high)
{
    int initial_value = active_high ? 0 : 1;

    me->line = gpiod_chip_get_line(chip, line_offset);
    if (NULL == me->line)                              { return PLATFORM_EIO; }
    if (gpiod_line_request_output(me->line,
                                  "led", initial_value) < 0) {
        return PLATFORM_EIO;
    }
    me->active_high = active_high;
    return led_base_init(&me->base, name, &led_gpio_ops);
}

static platform_err_t _led_gpio_on(struct led_base *me)
{
    struct led_gpio *gpio = (struct led_gpio *)me;
    if (gpiod_line_set_value(gpio->line,
                             gpio->active_high ? 1 : 0) < 0) {
        return PLATFORM_EIO;
    }
    return PLATFORM_EOK;
}
```

跟 stm32_full 的 `led_gpio.c` 比一下：

- stm32_full：`led_gpio.c` 调 `platform_pin_write(gpio->pin, ...)`，`platform_pin.c` 再调 `_g_ops->write`，`pin_board.c` 在板级 ops 里调 `HAL_GPIO_WritePin`，3 层
- linux_full：`led_gpio.c` 直接调 `gpiod_line_set_value(gpio->line, ...)`，0 层

中间层一个不留。理由就是 C.0 节那张表：libgpiod 已经是内核 platform 抽象暴露给用户的 OOP 接口，再套一层 `platform_pin_xxx` 是反工程。

### app/drivers/led/led_pwm.c（直接写 sysfs PWM）

```c
struct led_pwm {
    struct led_base base;
    int             chip_num, pwm_num;
    int             duty_fd, enable_fd;
    uint32_t        period_ns;
    uint8_t         brightness;
};

static platform_err_t _led_pwm_set_brightness(struct led_base *me,
                                              uint8_t level)
{
    struct led_pwm *pwm = (struct led_pwm *)me;
    pwm->brightness = level;

    if (me->is_on) {
        char buf[32];
        uint32_t duty_ns =
            (uint32_t)(((uint64_t)level * pwm->period_ns) / 255U);
        int n = snprintf(buf, sizeof(buf), "%u", duty_ns);
        if (lseek(pwm->duty_fd, 0, SEEK_SET) < 0)      { return PLATFORM_EIO; }
        if (write(pwm->duty_fd, buf, (size_t)n) < 0)   { return PLATFORM_EIO; }
    }
    return PLATFORM_EOK;
}
```

`led_pwm_init` 里走完 sysfs PWM 的标准开机流程：写 `/sys/class/pwm/pwmchipN/export`、写 period、`open` 出 duty_cycle / enable 两个 fd，后续 on/off/set_brightness 直接走 fd 写，一次性把 path 解析成 fd 比每次都 open 更省。

### app/drivers/led/led_i2c.c（直接走 i2c-dev）

```c
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

platform_err_t led_i2c_init(struct led_i2c *me, const char *name,
                            int bus_num, uint8_t dev_addr, uint8_t reg)
{
    char dev_path[32];
    snprintf(dev_path, sizeof(dev_path), "/dev/i2c-%d", bus_num);

    int fd = open(dev_path, O_RDWR);
    if (fd < 0)                                  { return PLATFORM_EIO; }
    if (ioctl(fd, I2C_SLAVE, dev_addr) < 0)      {
        close(fd); return PLATFORM_EIO;
    }
    me->fd = fd; me->dev_addr = dev_addr; me->reg = reg;
    me->val_on = 0x01; me->val_off = 0x00;
    return led_base_init(&me->base, name, &led_i2c_ops);
}

static platform_err_t _led_i2c_on(struct led_base *me)
{
    struct led_i2c *i2c = (struct led_i2c *)me;
    uint8_t buf[2] = { i2c->reg, i2c->val_on };
    if (write(i2c->fd, buf, 2) != 2) { return PLATFORM_EIO; }
    return PLATFORM_EOK;
}
```

`ioctl(fd, I2C_SLAVE, addr)` 把后续 read/write 转给这个从机，然后 `write` 两字节（寄存器地址 + 值）就完成了 SMBus byte write 等价操作。i2c-dev 文档里就是这个写法。

### app/environment_cfg/led_cfg.c（实例装配·去掉 INIT_ENV_EXPORT）

```c
int environment_init(void)
{
    _g_chip = gpiod_chip_open_by_name("gpiochip0");
    if (NULL == _g_chip)                                       { goto fail; }

    if (PLATFORM_EOK != led_gpio_init(&_led_status_inst, "status",
                                      _g_chip, 17, true))      { goto fail; }
    led_status = &_led_status_inst.base;

    if (PLATFORM_EOK != led_pwm_init(&_led_dimmer_inst, "dimmer",
                                     0, 0, 1000000U))           { goto fail; }
    led_dimmer = &_led_dimmer_inst.base;

    if (PLATFORM_EOK != led_i2c_init(&_led_panel_inst, "panel",
                                     1, 0x3C, 0x00))            { goto fail; }
    led_panel = &_led_panel_inst.base;

    if (PLATFORM_EOK != led_gpio_init(&_led_alarm_inst, "alarm",
                                      _g_chip, 22, true))       { goto fail; }
    led_alarm = &_led_alarm_inst.base;

    return 0;
fail:
    environment_exit();
    return -1;
}
```

跟 stm32_full 的 `led_cfg.c` 比：

- stm32_full：`INIT_ENV_EXPORT(_led_cfg)` 让链接器把这个函数登在 `__moduleExport5_start/_end` 段里，启动期 `platform_module_export_exec()` 自动跑
- linux_full：`environment_init()` 公开接口，main 显式调

为什么不沿用 GCC `__attribute__((constructor))`？因为这个工程的目标就是"应用层不需要任何隐式机制"。Linux 用户态的好处之一是控制流可见，所有装配什么时候发生写在 main 里，崩了一眼能看出哪一步错。`INIT_xxx_EXPORT` 是裸机时代的妥协（main 之前的链接器段），用户态没必要把它带过来。

### src/main.c

```c
int main(void)
{
    int ret = environment_init();
    if (ret != 0) {
        fprintf(stderr, "[main] environment_init failed, exit.\n");
        return 1;
    }

    struct led_base *all[4] =
        { led_status, led_dimmer, led_panel, led_alarm };

    for (int round = 0; round < 3 && _g_running; round++) {
        for (int i = 0; i < 4 && _g_running; i++) {
            led_on(all[i]);
            led_set_brightness(all[i], 128);
            sleep(1);
            led_off(all[i]);
        }
    }

    environment_exit();
    return 0;
}
```

应用层调用一字不知底下走的是 libgpiod / sysfs / i2c-dev 哪一个。`led_set_brightness` dispatch 到 PWM 子类时真生效，到 GPIO / I2C 子类时走父类默认 no-op。这是 OOP 多态在 Linux 用户态的完整演示。

## C.4 编译运行步骤

### 真机 Linux

```bash
sudo apt install libgpiod-dev          # Debian / Ubuntu / 树莓派 OS
# 或 sudo dnf install libgpiod-devel   # Fedora / RHEL

cd industrial/linux_full
make
sudo ./build/firmware
```

需要 sudo（或者把当前用户加到 `gpio` / `i2c` group + 相应 udev 规则）。3 圈演示后退出，0 警告 0 错误。

### 不在 Linux 上 / 没装 libgpiod-dev

只能跑 syntax check，验证 .c 文件语法和头文件依赖合法：

```bash
make check-syntax
```

这个 target 用 `syntax_stubs/` 下的占位 `<gpiod.h>` / `<linux/i2c-dev.h>` / `<sys/ioctl.h>` 让 `gcc -fsyntax-only` 跑过。**只是开发期校对工具**，不能链接成可执行文件。要跑必须在装好 libgpiod-dev 的 Linux SBC 上 `make`。

### 接线

| 句柄 | BCM 编号 | 树莓派 4B 物理引脚 | 接法 |
|---|---|---|---|
| `led_status` | GPIO17 | 11 | LED 长脚串 220Ω 接 GPIO17，短脚接 GND |
| `led_alarm` | GPIO22 | 15 | 同上 |
| `led_dimmer` | pwmchip0/pwm0 | 12 | 树莓派需要在 `/boot/config.txt` 加 `dtoverlay=pwm` 启用硬件 PWM |
| `led_panel` | i2c-1 (SDA=2, SCL=3) | 3 / 5 | 接一颗 PCA9554 / TCA6408 这类 I/O expander |

GPIO17 / GPIO22 这两颗 LED 不需要任何外设配置，硬件接好就能跑。PWM / I2C 那两颗需要先在 device tree / config.txt 里启用，否则 `/sys/class/pwm/pwmchip0` 或 `/dev/i2c-1` 不存在，对应子类的 init 会返回 `PLATFORM_EIO`。

板子换了改 `app/environment_cfg/led_cfg.c` 顶部的常量：

```c
#define LED_GPIO_CHIP_NAME    "gpiochip0"
#define LED_STATUS_LINE       17
#define LED_ALARM_LINE        22
#define LED_PWM_CHIP          0
#define LED_PWM_NUM           0
#define LED_I2C_BUS           1
#define LED_I2C_ADDR          0x3C
```

香橙派 5 上 `gpiochip` 编号、PWM chip 编号都不一样，`gpiodetect` 命令列出板上所有 GPIO 控制器，挑写在板子上 LED 实际接的那一个。

## C.5 这个工程跑通了什么

跑 `sudo ./build/firmware`，3 圈演示，每一圈：

1. `led_on(led_status)` → `_led_gpio_on` 调 `gpiod_line_set_value(line, 1)`
2. `led_on(led_dimmer)` → `_led_pwm_on` 写 duty_fd（占空时间） + 写 enable_fd（"1"）
3. `led_set_brightness(led_dimmer, 128)` → 父类 dispatch 到 `_led_pwm_set_brightness`，PWM 子类 duty 改成 128/255 比例
4. `led_set_brightness(led_status, 128)` → 父类 dispatch 看到 GPIO 子类没填 `set_brightness`，走父类默认 no-op
5. `led_on(led_panel)` → `_led_i2c_on` 调 `write(fd, [reg, 0x01], 2)`
6. `led_on(led_alarm)` → 同 led_status，line 22

三种子类 dispatch 路径全部跑完，应用层调用一字不知底下走哪一条。**OOP 抽象保留，platform 抽象层去掉**。

按全书 18 章过一遍：

| 章节 | 抽象 | 在这个工程里的位置 |
|---|---|---|
| ch01 | struct + me | 所有 `led_xxx` 函数第一参数 `struct led_base *me` |
| ch02 | 信息隐藏 | `struct led_gpio` / `led_pwm` / `led_i2c` 字段定义在子类头文件，应用层只看到 `struct led_base *` |
| ch03 | class 化 | `led_gpio_init / led_on / led_off / led_set_brightness` 命名前缀统一 |
| ch04 | 数据归位 | 静态实例 `_led_status_inst`（数量固定的全局对象选静态实例） |
| ch05 | HAL 漫游 | libgpiod 这一片就是漫游目标，它的内部就是 ioctl 到内核 gpio chardev |
| ch06 | 继承痛点 | `struct led_gpio` 把 `struct led_base` 嵌在第一字段 |
| ch07 | 函数指针 | `struct led_ops` 里的函数指针 |
| ch08 | 函数指针传参 | 同 ch07 |
| ch09 | ops 表雏形 | `struct led_ops` 三个字段 |
| ch10 | ops 放进对象 | `struct led_base.ops` 字段 |
| ch11 | 多态完整 | `led_on(led_status)` / `led_on(led_dimmer)` / `led_on(led_panel)` 调用一字不差，底下走 GPIO / PWM / I2C 三种实现 |
| ch12 | 向上转型 | `led_status = &_led_status_inst.base;` |
| ch13 | container_of | base 在第一字段时简化为 `(struct led_gpio *)me` |
| ch14 | 纯虚 | `struct led_ops` 里 `on / off` pure virtual；`set_brightness` 选填 |
| ch15 | 换硬件不改应用 | 跟 stm32_full 的 led_base.[hc] / environment_export.h 字节级一致：同一组 OOP 抽象，下面 STM32 自抽 platform，Linux 直接用 libgpiod / sysfs / i2c-dev |
| ch16 | Linux 风格 | 这一份就是 Linux 用户态原生写法 |
| ch17 | 链接初始化 | **这个工程没有**：Linux 用户态用 `main` 里显式 `environment_init()` 比 GCC ctor 更直接，控制流可见 |

## C.6 跟 stm32_full 的对比（关键表）

linux_full 没有 platform 抽象层，按 §C.0 决策，led 子类直接调内核暴露的 sysfs / i2c-dev / libgpiod 接口；stm32_full 因为 HAL 不是设备模型，必须在中间补一层 `platform_xxx` 才走得到 HAL。这张表把两边的 GPIO / PWM / I2C 三条接口逐行对照，能直观看出 platform 层的边界在哪。

| 维度 | stm32_full（裸机） | linux_full（用户态） |
|---|---|---|
| platform 抽象层 | **必须自抽**（HAL 不是设备模型） | **禁自抽**（内核已做完） |
| GPIO 接口 | `platform_pin_write` → `pin_board.c` → `HAL_GPIO_WritePin` | `led_gpio.c` 直接 `gpiod_line_set_value`（无 platform 层） |
| PWM 接口 | `platform_pwm_set_duty` → `pwm_board.c` → `HAL_TIM_PWM_xxx` | `led_pwm.c` 直接 write `/sys/class/pwm/pwmchipN/pwmM/duty_cycle`（无 platform 层，内核做完了） |
| I2C 接口 | `platform_i2c_transfer` → `i2c_board.c` → `HAL_I2C_Master_Transmit`（bus + client 二层） | `led_i2c.c` 直接 `open(/dev/i2c-N)` + `ioctl(I2C_SLAVE)` + `write`（无 platform 层） |
| 启动期初始化 | linker section + `module_export_exec()` 7 级 | `main` 里显式 `environment_init()` |
| 错误码 | `platform_err_t` 11 个 enum | `platform_err_t` 4 个 enum（对齐 POSIX errno） |
| 主循环延时 | busy-wait `for (volatile i ...)` | `sleep(1)` |
| 信号处理 | NVIC 中断 | `signal(SIGINT, ...)` |
| Make 目标 | `make` + `make flash` | `make` + `sudo ./build/firmware` |

OOP 抽象（`struct led_base + 多子类 + 设备句柄统一导出`）两边字节级一致：`led_base.h` 接口、`environment_export.h` 句柄声明、main 里的调用形态。**变化只在 platform 抽象层和板级实现**。这一对比就是这本书工程判断力教学的核心。

## C.7 进阶练习

**练习 1**：加一个 SPI LED 子类，直接走 spidev（`/dev/spidevN.M` + `ioctl(SPI_IOC_MESSAGE)`）。建文件 `app/drivers/led/led_spi.h / .c`，参照 `led_i2c.c` 的写法。底下不要套任何 platform 抽象层，spidev 已经是内核 spi subsystem 暴露给用户态的 OOP 接口。

**练习 2**：加一个温度 sensor 子类，直接走 iio 子系统（`/sys/bus/iio/devices/iio:deviceN/in_temp_raw`）。新建 `app/drivers/sensor/`，先抽 `struct sensor_base + struct sensor_ops` 父类，再写 `struct sensor_iio` 子类直接读 sysfs。这一组练习帮你把"OOP 抽象保留 / platform 抽象去掉"的判断力推广到温度、压力、湿度、加速度计这一大类设备。

**练习 3**：把 4 颗 LED 装配从 `led_cfg.c` 的硬编码常量改成读 `/etc/zhaoming-leds.conf` 配置文件。`environment_init()` 启动期 `fopen` + 解析。这一组练习把 stm32_full 的 device tree 装配等价物搬到 Linux 用户态。

**练习 4**：加按钮（GPIO 输入）。新增 `app/drivers/button/`，子类直接调 `gpiod_line_request_input` + `gpiod_line_event_wait` + `gpiod_line_event_read`。在 main 里把按钮绑定到"按下切换 LED 模式"。这是 libgpiod 1.x 的 event API，2.x 改成 line_request 配 edge_event_buffer，移植路径直白。

## C.8 已知工程性细节

- **真机要 sudo**（或者用户加 `gpio` / `i2c` group + 配 udev 规则）。`/dev/gpiochip0` / `/dev/i2c-1` 默认只 root 能开。
- **gpiochip 编号在不同 SBC 不一样**：树莓派 4B 是 `gpiochip0`，香橙派 5 是 `gpiochip1`（sunxi-gpio），飞腾派 D2000 是 `gpiochip0`。`gpiodetect` 命令列出板上所有 GPIO 控制器。
- **sysfs PWM 不是所有 SBC 都开**：树莓派 4B 默认 device tree 不启用硬件 PWM，要在 `/boot/config.txt` 加 `dtoverlay=pwm` 或 `dtoverlay=pwm-2chan`。`/sys/class/pwm/` 不存在就是这个原因。
- **libgpiod 1.x vs 2.x API 差异**：本工程用 1.x（Debian 12 / Ubuntu 22.04 / 树莓派 OS Bookworm 之前默认）。Bookworm 之后默认装 2.x，API 改成 request builder 风格，移植到 2.x 需要改 `led_gpio_init` 里两行调用。
- **i2c-dev 模块**：树莓派要在 `/boot/config.txt` 加 `dtparam=i2c_arm=on` 启用 I2C bus 1。香橙派 5 默认开 i2c-1 / i2c-3 / i2c-5 几条，看接的哪条。

## 下一篇

[附录 D · 配套代码索引 + 视频清单 + 编译运行指南](D-配套代码索引.md)
