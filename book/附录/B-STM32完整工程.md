# 附录 B · STM32 完整工程：跑通全书所有抽象

参考代码：[`industrial/stm32_full/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/industrial/stm32_full/)。

附录 B 和附录 C 是这本书的"集大成"工程。每章配套的 PC 模拟代码已经把每个概念讲透了，但读者会有一个合理的诉求：**能不能给我一个跑在真实 MCU 上的完整工程，里面所有抽象（struct + me、ops 表、container_of、platform_pin、initcall）一次跑通？**

这两个附录就是给你这个工程。附录 B 是 STM32（Cortex-M 系列），附录 C 是 Linux 用户态（ARM64 SBC）。

> 工程参照真实工业项目（同款 STM32H7 Cortex-M7 控制板）的代码组织方式整理而成。引脚配置 / HAL 参数按 STM32F407 Discovery 这块流通最广的开发板写出，换其他 STM32 系列（H7 / F1 / L4 / G0 等）只改 `arch/board/pin_board.c` 一份文件，应用层和驱动层一行不动。

## B.1 工程概览

目标硬件：**STM32F407 Discovery 板**（Cortex-M4F，168 MHz，1 MB Flash，192 KB SRAM）。这是国内最容易拿到的开发板之一，淘宝几十块到一百多块。

不同 MCU 跑这个工程都是 5% 改动量。如果你用的是 STM32H7、F1、L4 系列任何一颗，照着 `app/platform/arch/board/` 改一份文件就能跑。

工程实现的功能：板上 4 颗 LED（绿、橙、红、蓝）按 1 秒间隔流水灯。看起来简单，但代码里跑通了全书所有 OOP 抽象：

- ch01 封装：`struct led_base + me` 指针
- ch10/11 多态：`led_base_ops_t + ops` 表 + vptr
- ch12 向上转型：`led_gpio_t { led_base_t base; ... }`，`(led_base_t *)led_gpio` 直接转
- ch13 container_of：从基类指针拿回 GPIO 子类的私有字段（base 在第一字段时简化为 `(led_gpio_t *)me`）
- ch14 纯虚：`led_base_ops_t` 里 `led_on / led_off` 是 pure virtual，子类必须实现
- ch15 platform 抽象到底：`platform_pin_xxx()` 封装函数体内走 ops dispatch，**字符串 pin 名**让上层永远看不到 port 索引
- ch17 链接自动初始化：`INIT_BOARD_EXPORT` / `INIT_ENV_EXPORT` 宏注册所有初始化项，启动时按 7 级顺序自动跑

## B.2 工程结构

附录 B 的工程结构脱胎于真实工业项目（脱敏后保留教学需要的最小集合）。三层信息可见性差别是工业纪律的核心：

| 层 | 看得到的内容 | 看不到的内容 |
|---|---|---|
| **应用层** (`Core/Src/main.c` / `mock/main_pc.c`) | `led_base_t *` 句柄 | 子类完整类型、ops 表、平台细节 |
| **驱动层** (`app/drivers/led/`) | `led_gpio_t`、`platform_pin_xxx()` 封装函数 | `platform_pin_ops_t`、寄存器、HAL |
| **平台层** (`app/platform/`、`app/platform/arch/board/`) | 所有 ops 表、寄存器、HAL | / |

```
industrial/stm32_full/
├── README.md
├── Makefile                                 # 双模 (真机 + MOCK)
├── linker_stm32f407.ld                      # 链接脚本 (含 8 个 moduleExport 段)
├── startup_stm32f407xx.s                    # 启动汇编骨架
├── Core/                                    # CubeMX 风格的 MCU 启动入口
│   ├── Inc/main.h
│   └── Src/main.c                           # 真机 main: HAL_Init + ... + module_export_exec
├── app/                                     # 应用层
│   ├── project_config.h                     # PLATFORM_OS / PLATFORM_HEAP_ENABLE 开关
│   ├── platform/                            # 平台抽象层
│   │   ├── platform_def.h                   # 跨编译器宏 + platform_err_t + container_of
│   │   ├── platform_pin.h / .c              # PIN 框架 (字符串名 + ops 分发)
│   │   ├── platform_assert.h / .c
│   │   ├── platform_module_export.h / .c    # 8 级 INIT_xxx_EXPORT (ARMCC/IAR/GCC + MOCK)
│   │   └── arch/board/
│   │       └── pin_board.c                  # STM32 pin 子类 (依赖 HAL)
│   ├── drivers/
│   │   └── led/
│   │       ├── led_base.h / .c
│   │       └── led_gpio.h / .c
│   └── environment_cfg/
│       └── led_cfg.c                        # 4 颗 LED 实例 + INIT_ENV_EXPORT
└── mock/                                    # PC 模拟模式
    ├── main_pc.c
    └── pin_board_pc.c
```

工业纪律的硬规则：

- **驱动层** `#include "platform/platform_pin.h"` 调封装函数（普通 C 函数），永远不直接碰 GPIO 寄存器或 HAL
- **平台层框架** `platform_pin.c` 维护 `static const platform_pin_ops_t *_g_ops` 指针，子类通过 `platform_pin_register(&xxx_ops)` 启动期填进来
- **字符串 pin 名**（`"PA.5"` / `"PD.12"`）：调用方写字面字符串，看不到 port 索引或寄存器地址。换芯片只改 `pin_board.c` 的字符串解析

## B.3 关键文件

> **API 演化说明**：教学章节 ch01-ch15 用的是 `platform_gpio_init/write/read/deinit(uint8_t pin, ...)` 函数式 API（一组独立函数 + 数字 pin 编号），是**简化形态**便于教学引入。
>
> 附录 B 工程演化到工业级形态：
>
> - API 改名为 `platform_pin_init/write/read(int32_t pin, ...)`，跟 RT-Thread / 国内多数控制板项目命名一致
> - **字符串 pin 名**：调用方写 `"PA.5"` / `"PD.12"` / `"PI.14"`，让 platform 层内部解析。换芯片只改解析表，driver 一字不动
> - 错误码用统一的 `platform_err_t` 枚举（`PLATFORM_EOK / PLATFORM_EINVAL / ...`），所有 init 函数返回它
>
> 这一组演化跟 ch15 讲的"对外封装、对内 ops"是一回事，只是从最简函数式逐步推到工业版形态。读者把附录 B 工程跟 ch15 的最终形态对比，能直观看出"工业代码长什么样"。

### app/platform/platform_def.h（跨编译器宏 + 错误码 + container_of）

```c
typedef enum
{
    PLATFORM_EOK       =  0,    /**< 没错 */
    PLATFORM_ERROR     = -1,    /**< 通用错误 */
    PLATFORM_ETIMEOUT  = -2,    /**< 超时 */
    PLATFORM_EFULL     = -3,    /**< 资源已满 */
    PLATFORM_EEMPTY    = -4,    /**< 资源已空 */
    PLATFORM_ENOMEM    = -5,    /**< 内存不足 */
    PLATFORM_ENOSYS    = -6,    /**< 不支持 / 未实现 */
    PLATFORM_EBUSY     = -7,    /**< 忙 */
    PLATFORM_EIO       = -8,    /**< IO 错误 */
    PLATFORM_EINTR     = -9,    /**< 系统调用被中断 */
    PLATFORM_EINVAL    = -10    /**< 无效参数 */
} platform_err_t;

#ifndef container_of
#define container_of(ptr, type, member)                  \
    ({                                                   \
        void *__mptr = (void *)(ptr);                    \
        ((type *)(__mptr - offsetof(type, member)));     \
    })
#endif

/* 跨编译器 attribute 宏 */
#if defined(__CC_ARM) || defined(__CLANG_ARM)
    #define PLATFORM_SECTION(x)         __attribute__((section(x)))
    #define PLATFORM_USED               __attribute__((used))
    /* ... */
#elif defined(__IAR_SYSTEMS_ICC__)
    #define PLATFORM_SECTION(x)         @ x
    #define PLATFORM_USED               __root
    /* ... */
#elif defined(__GNUC__)
    #define PLATFORM_SECTION(x)         __attribute__((section(x)))
    #define PLATFORM_USED               __attribute__((used))
    /* ... */
#else
    #error not supported tool chain
#endif
```

`platform_err_t` 是工程通用错误码。所有 init / register 函数返回它，调用方用 `if (PLATFORM_EOK != ret) goto exit;` 集中错误处理。

`container_of` 是 Linux 内核 1990 年代引入的标准宏（ch13 详细讲过）。这里复用同款实现。

跨编译器宏统一三大工业工具链（ARM Compiler 5/6、IAR、GCC）的 `__attribute__((section/used/aligned/weak))` 拼写，让上层代码一字不改就能在三种编译器下编。

### app/platform/platform_pin.h（PIN 框架对外 API）

```c
/* PIN 电平 */
#define PIN_LOW                       0x00
#define PIN_HIGH                      0x01

/* PIN 工作模式 */
#define PIN_MODE_OUTPUT               0x00
#define PIN_MODE_INPUT                0x01
#define PIN_MODE_INPUT_PULLUP         0x02
/* ... */

/* ops 表抽象 (子类填写) */
typedef struct
{
    void (*pin_mode)(int32_t pin, int32_t mode);
    void (*pin_write)(int32_t pin, int32_t value);
    int32_t (*pin_read)(int32_t pin);
    /* ... IRQ ops */
    int32_t (*pin_get)(const char *name);
} platform_pin_ops_t;

/* 注册接口 (子类用) */
platform_err_t platform_pin_register(const platform_pin_ops_t *ops);

/* 公共 API (上层调) */
void platform_pin_mode(int32_t pin, int32_t mode);
void platform_pin_write(int32_t pin, int32_t value);
int32_t platform_pin_read(int32_t pin);

/* "PA.5" / "PD.12" / "PI.14" -> pin_num */
int32_t platform_pin_get(const char *name);
```

驱动层只 include 这一份，调的是普通 C 函数。看不到 ops 字段、看不到寄存器、看不到 HAL。

### app/platform/platform_pin.c（封装函数 + 内部 ops 分发）

```c
static const platform_pin_ops_t *_g_ops = NULL;

platform_err_t platform_pin_register(const platform_pin_ops_t *ops)
{
    platform_err_t ret = PLATFORM_EINVAL;

    if (NULL == ops)
    {
        goto exit;
    }
    _g_ops = ops;
    ret = PLATFORM_EOK;

exit:
    return ret;
}

void platform_pin_mode(int32_t pin, int32_t mode)
{
    platform_assert(_g_ops != NULL);
    platform_assert(_g_ops->pin_mode != NULL);
    _g_ops->pin_mode(pin, mode);
}

void platform_pin_write(int32_t pin, int32_t value)
{
    platform_assert(_g_ops != NULL);
    platform_assert(_g_ops->pin_write != NULL);
    _g_ops->pin_write(pin, value);
}

int32_t platform_pin_get(const char *name)
{
    int32_t ret = -1;

    platform_assert(_g_ops != NULL);
    platform_assert(name != NULL);
    platform_assert(name[0] == 'P');

    if (NULL == _g_ops->pin_get)
    {
        ret = PLATFORM_ENOSYS;
        goto exit;
    }
    ret = _g_ops->pin_get(name);

exit:
    return ret;
}
```

`_g_ops` 是 platform 层的私有状态（`static`，文件作用域），外部链接不到。这就是 ch15 讲的"对外封装、对内 ops"形态。换硬件只改一个指针，封装函数签名永远不变。

### app/platform/arch/board/pin_board.c（STM32 pin 子类，节选）

子类实现 4 个 ops 函数，启动期通过 `INIT_BOARD_EXPORT` 自动注册：

```c
#define PIN_NUM(port, no)        (((((port) & 0xFu) << 4) | ((no) & 0xFu)))
#define PIN_PORT(pin)            ((uint8_t)(((pin) >> 4) & 0xFu))
#define PIN_NO(pin)              ((uint8_t)((pin) & 0xFu))
#define PIN_STPORT(pin)          ((GPIO_TypeDef *)(GPIOA_BASE + (0x400u * PIN_PORT(pin))))
#define PIN_STPIN(pin)           ((uint16_t)(1u << PIN_NO(pin)))

static int32_t _stm32_pin_get(const char *name)
{
    /* 解析 "PA.5" / "PD.12" 这种字面字符串, 高位 port + 低位 num. */
    int hw_port_num = (int)(name[1] - 'A');
    int hw_pin_num  = 0;
    int name_len    = strlen(name);

    for (int i = 3; i < name_len; i++)
    {
        hw_pin_num *= 10;
        hw_pin_num += name[i] - '0';
    }
    return PIN_NUM(hw_port_num, hw_pin_num);
}

static void _stm32_pin_write(int32_t pin, int32_t value)
{
    if (PIN_PORT(pin) >= __STM32_PORT_MAX) goto exit;
    HAL_GPIO_WritePin(PIN_STPORT(pin), PIN_STPIN(pin),
                      (GPIO_PinState)value);

exit:
    return;
}

/* ... pin_mode / pin_read 同样基于 HAL_GPIO_xxx 实现 */

static const platform_pin_ops_t _stm32_pin_ops =
{
    .pin_mode  = _stm32_pin_mode,
    .pin_write = _stm32_pin_write,
    .pin_read  = _stm32_pin_read,
    .pin_get   = _stm32_pin_get,
    /* IRQ ops 在工业版补全, 教学版留 NULL */
};

static void _platform_hw_pin_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /* ... 所有 port 时钟使能 */
    platform_pin_register(&_stm32_pin_ops);
}
INIT_BOARD_EXPORT(_platform_hw_pin_init);
```

`PIN_NUM(port, num)` 把 port (A=0, B=1, ...) 高 4 位 + 引脚号 (0-15) 低 4 位编码成单参数 `int32_t`。这一招让 `platform_pin_xxx` 的签名永远只有一个 `pin` 参数，无论底下是单 port 的小芯片还是 12 个 port 的大芯片。

跟 Linux 内核 `gpio_set_value(unsigned int gpio, ...)`、Zephyr `gpio_pin_set_dt(spec, value)` 是同一种工业纪律：**对外是单参数（或句柄），port 信息藏在编码里**。

### app/platform/platform_module_export.h（8 级 initcall 宏）

```c
typedef struct
{
    void (*func)(void);
} module_export_t;

#ifdef MOCK_PC
/* PC mock: 用 GCC ctor 直接在 main 之前自动跑 */
#define _PLATFORM_INIT_CTOR(_func, _prio)                            \
    __attribute__((constructor(_prio)))                              \
    static void _func##_module_init_ctor(void) { _func(); }

#define INIT_BOARD_EXPORT(fn)            _PLATFORM_INIT_CTOR(fn, 101)
#define INIT_PREV_EXPORT(fn)             _PLATFORM_INIT_CTOR(fn, 102)
#define INIT_DEVICE_EXPORT(fn)           _PLATFORM_INIT_CTOR(fn, 103)
/* ... 共 8 级 */

#else  /* 真机三大编译器 */

#define INIT_EXPORT(_func, level)                                   \
    MODULE_EXPORT_USED const module_export_t                        \
    module_init_##_func MODULE_EXPORT_SECTION("moduleExport" level) = \
    {                                                               \
        .func = _func,                                              \
    }

#define INIT_BOARD_EXPORT(fn)            INIT_EXPORT(fn, "1")
#define INIT_PREV_EXPORT(fn)             INIT_EXPORT(fn, "2")
#define INIT_DEVICE_EXPORT(fn)           INIT_EXPORT(fn, "3")
#define INIT_COMPONENT_EXPORT(fn)        INIT_EXPORT(fn, "4")
#define INIT_ENV_EXPORT(fn)              INIT_EXPORT(fn, "5")
#define INIT_APP_EXPORT(fn)              INIT_EXPORT(fn, "6")
#define INIT_SYSTEM_READY_EXPORT(fn)     INIT_EXPORT(fn, "7")
#define UNIT_TEST_EXPORT(fn)             INIT_EXPORT(fn, "8")

#endif

extern void platform_module_export_exec(void);
extern void platform_unit_test_exec(void);
```

7 个优先级覆盖一个工业项目从板级到应用层的所有启动顺序，第 8 级 `UNIT_TEST_EXPORT` 单独留给单元测试。

跨编译器（ARMCC `$$Base/$$Limit`、IAR `__section_begin`、GCC `_start/_end`）通过条件编译统一接口。MOCK 模式用 `__attribute__((constructor(N)))` 直接让 GCC 在 main 之前自动跑，**应用层调 `platform_module_export_exec()` 在两种模式下调用形态完全一致**。

### app/drivers/led/led_base.h / .c（基类）

```c
/* led_base.h */
struct led_base_ops;

typedef struct
{
    struct led_base_ops *ops;
} led_base_t;

typedef struct led_base_ops
{
    /* pure virtual function: 子类必须填写 */
    void (*led_on)(led_base_t *me);
    void (*led_off)(led_base_t *me);
} led_base_ops_t;

void led_on(led_base_t *me);
void led_off(led_base_t *me);
```

```c
/* led_base.c */
#include "led/led_base.h"

void led_on(led_base_t *me)
{
    me->ops->led_on(me);
}

void led_off(led_base_t *me)
{
    me->ops->led_off(me);
}
```

`led_on` / `led_off` 是**基类层封装函数**，函数体只做一件事：通过 ops 表 dispatch 到子类实现。这是 ch11 讲的"基类层包装函数"形态。

### app/drivers/led/led_gpio.h / .c（GPIO 子类）

```c
/* led_gpio.h */
typedef struct
{
    led_base_t base;        /* 基类放第一字段, 上转直接 cast */
    int32_t    pin_num;
    bool       light_level;
} led_gpio_t;

platform_err_t led_gpio_init(
    led_gpio_t *me, const char *pin_name, bool light_level);
```

```c
/* led_gpio.c */
#include "led/led_gpio.h"
#include "platform/platform_pin.h"

static void _led_gpio_on(led_base_t *me);
static void _led_gpio_off(led_base_t *me);

static const led_base_ops_t _ops =
{
    .led_on  = _led_gpio_on,
    .led_off = _led_gpio_off,
};

platform_err_t led_gpio_init(
    led_gpio_t *me, const char *pin_name, bool light_level)
{
    platform_err_t ret = PLATFORM_EOK;
    int32_t pin_num;

    if ((NULL == me) || (NULL == pin_name))
    {
        ret = PLATFORM_EINVAL;
        goto exit;
    }
    pin_num = platform_pin_get(pin_name);
    if (pin_num < 0)
    {
        ret = PLATFORM_EINVAL;
        goto exit;
    }
    platform_pin_mode(pin_num, PIN_MODE_OUTPUT);
    platform_pin_write(pin_num, !light_level);

    me->pin_num     = pin_num;
    me->light_level = light_level;
    me->base.ops    = (led_base_ops_t *)&_ops;

exit:
    return ret;
}

static void _led_gpio_on(led_base_t *me)
{
    led_gpio_t *self = (led_gpio_t *)me;
    platform_pin_write(self->pin_num, self->light_level);
}

static void _led_gpio_off(led_base_t *me)
{
    led_gpio_t *self = (led_gpio_t *)me;
    platform_pin_write(self->pin_num, !self->light_level);
}
```

子类的 ops 表 `_ops` 是 `static const`，所有 `led_gpio_t` 实例共享同一张表（在 Flash 里）。这是 ch10 讲的"vtable 在 Flash、vptr 在 RAM"内存布局。

`(led_gpio_t *)me` 直接把 base 指针转回子类，因为 `base` 在第一字段。等价的 Linux 内核风格写法：

```c
static void _led_gpio_on(led_base_t *me)
{
    led_gpio_t *self = container_of(me, led_gpio_t, base);
    /* ... */
}
```

两种写法机器码一字不差。`base` 在第一字段时偏移 0，`container_of` 减去 0 等于直接 cast。

### app/environment_cfg/led_cfg.c（4 颗 LED 实例配置）

```c
#include "platform/platform_module_export.h"
#include "led/led_gpio.h"

led_base_t *led_green   = NULL;
led_base_t *led_orange  = NULL;
led_base_t *led_red     = NULL;
led_base_t *led_blue    = NULL;

static led_gpio_t _led_green_inst;
static led_gpio_t _led_orange_inst;
static led_gpio_t _led_red_inst;
static led_gpio_t _led_blue_inst;

static void _led_cfg(void)
{
    platform_err_t ret;

    ret = led_gpio_init(&_led_green_inst,  "PD.12", true);
    if (PLATFORM_EOK != ret) goto exit;
    led_green = (led_base_t *)&_led_green_inst;

    ret = led_gpio_init(&_led_orange_inst, "PD.13", true);
    if (PLATFORM_EOK != ret) goto exit;
    led_orange = (led_base_t *)&_led_orange_inst;

    ret = led_gpio_init(&_led_red_inst,    "PD.14", true);
    if (PLATFORM_EOK != ret) goto exit;
    led_red = (led_base_t *)&_led_red_inst;

    ret = led_gpio_init(&_led_blue_inst,   "PD.15", true);
    if (PLATFORM_EOK != ret) goto exit;
    led_blue = (led_base_t *)&_led_blue_inst;

exit:
    return;
}
INIT_ENV_EXPORT(_led_cfg);
```

注意：

- 4 颗 LED 全是 `led_base_t *` 类型对外暴露，应用层只看到接口
- 启动时 `INIT_ENV_EXPORT` 自动调用 `_led_cfg`，不用手动写在 main 里
- 改板子只改这一份的 4 个 pin 名字符串，**应用层 main.c 一行不改**

这就是这本书要教给你的开闭原则（Open/Closed Principle）：对扩展开放、对修改关闭。

### Core/Src/main.c（真机入口，CubeMX 风格）

```c
#include "main.h"
#include "led/led_base.h"
#include "platform/platform_module_export.h"

extern led_base_t *led_green;
extern led_base_t *led_orange;
extern led_base_t *led_red;
extern led_base_t *led_blue;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

static void delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms * 16800UL; i++)
        ;
}

int main(void)
{
    led_base_t *all[4];
    uint32_t cur = 0;

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    /* 一行调用所有 INIT_xxx_EXPORT 注册的函数 */
    platform_module_export_exec();

    all[0] = led_green;
    all[1] = led_orange;
    all[2] = led_red;
    all[3] = led_blue;

    while (1)
    {
        for (uint32_t i = 0; i < 4; i++)
        {
            led_off(all[i]);
        }
        led_on(all[cur]);
        cur = (cur + 1) % 4;
        delay_ms(1000);
    }
}
```

`main` 一共 25 行。所有"哪里初始化 LED"、"GPIO 怎么配"、"用哪个 port"的复杂度都被抽象层吸收。

### linker_stm32f407.ld（链接脚本骨架）

```ld
ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
    CCMRAM (rw) : ORIGIN = 0x10000000, LENGTH = 64K
}

_estack = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)

        . = ALIGN(4);
        moduleExport1_start = .;
        KEEP(*(moduleExport1))
        moduleExport1_end = .;

        . = ALIGN(4);
        moduleExport2_start = .;
        KEEP(*(moduleExport2))
        moduleExport2_end = .;

        /* ... 3, 4, 5, 6, 7, 8 同样 ... */

        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .data : AT(_etext) { /* ... copy from Flash to SRAM ... */ } > SRAM
    .bss  :             { /* ... clear at startup ... */         } > SRAM
}
```

要点：

- 8 个 moduleExport 段嵌在 `.text` 段之后，每段对应 `INIT_xxx_EXPORT` 一个优先级
- `KEEP(...)` 防止 linker 把没人显式引用的 export 项误删（initcall 段必须完整保留）
- `moduleExportN_start` / `moduleExportN_end` 符号是 linker 生成的边界，给 `platform_module_export.c` 的 GCC 分支用

完整 CubeMX 生成的脚本里还有 `_Min_Heap_Size` / `_Min_Stack_Size` / `.preinit_array` / `.init_array` / `.ARM.exidx` 等段。真机移植时把 CubeMX 生成版整段抄进来，在 `.text` 段后插入这 8 个 `moduleExport` 段即可。

## B.4 编译运行步骤

### PC 模拟模式（不依赖 STM32 HAL）

直接验证整套抽象的业务逻辑：

```bash
cd industrial/stm32_full
make MOCK=1
./build/firmware-pc.exe       # Windows
./build/firmware-pc           # Linux/macOS
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

3 圈流水灯（12 步）后退出。MOCK 模式下：

- `INIT_BOARD_EXPORT(_platform_hw_pin_init)` 通过 `__attribute__((constructor(101)))` 在 main 之前自动跑，注册 PC pin ops
- `INIT_ENV_EXPORT(_led_cfg)` 通过 `constructor(105)` 自动跑，装配 4 颗 LED 实例
- main 调 `platform_module_export_exec()` 是 nop（ctor 已经跑过）
- 调用形态、应用层代码、驱动层代码跟真机模式**完全一致**

### 真机模式（STM32F407 Discovery）

环境要求：

- arm-none-eabi-gcc 工具链（搜索引擎搜"ARM GNU Toolchain"到 ARM 官网下载）
- STM32F407 Discovery 板（或任意 STM32 板，改 `pin_board.c` 即可）
- ST-Link 调试器（板上自带）

工程准备：

```bash
# 1. 用 STM32CubeMX 生成 STM32F407VGTx 工程, 把生成的 Drivers/ 拷贝到 vendor/
cp -r ${CUBEMX_PROJECT}/Drivers/STM32F4xx_HAL_Driver vendor/
cp -r ${CUBEMX_PROJECT}/Drivers/CMSIS                vendor/

# 2. 把 CubeMX 生成的 SystemClock_Config + MX_GPIO_Init 抄进 Core/Src/main.c
#    把 startup_stm32f407xx.s 替换为 CubeMX 生成版
```

编译 + 烧录：

```bash
make
make flash    # 用 st-flash 烧到 0x08000000
```

预期：板上 4 颗 LED 依次亮灭，1 秒切一颗。

如果你的板子 LED 引脚不一样：改 `app/environment_cfg/led_cfg.c` 里 4 个 `"PD.NN"` 字符串。如果 LED 是低电平点亮：把 `led_gpio_init(..., true)` 的 `true` 改成 `false`。其他文件一字不改。

如果你换成 STM32F1 / F4 / H7 别的型号：复制 `app/platform/arch/board/pin_board.c` 改头文件名（`stm32xxxx_hal.h`）和时钟使能宏，应用层 + driver 层一字不动。

## B.5 这个工程跑通了什么

按全书 18 章过一遍：

| 章节 | 抽象 | 在这个工程里的位置 |
|---|---|---|
| ch01 | struct + me | 所有 `led_xxx` 函数第一参数 `led_base_t *me` |
| ch02 | 信息隐藏 | `led_gpio_t` 字段定义在 `led_gpio.h`，应用层只看到 `led_base_t *` |
| ch03 | class 化 | `led_gpio_init / led_on / led_off` 命名前缀统一 |
| ch04 | 数据归位 | 静态实例 `_led_green_inst` 装板上 4 颗 LED（数量固定的全局对象选静态实例，跟 ch04 4.7.2 节"按场景选"一致） |
| ch05 | HAL 漫游 | `pin_board.c` 里 `HAL_GPIO_WritePin` 调用 |
| ch06 | 继承痛点 | `led_base_t.ops` 字段被两种子类（GPIO / PWM 等）共享 |
| ch07 | 函数指针 | `led_base_ops_t` 里的 `void (*led_on)(led_base_t *)` |
| ch08 | 函数指针传参 | `platform_pin_register(&_stm32_pin_ops)` 把整张 ops 表当参数传 |
| ch09 | ops 表雏形 | `platform_pin_ops_t` |
| ch10 | ops 放进对象 | `led_base_t.ops` 字段 |
| ch11 | 多态完整 | `led_on(led_blue)` 跟 `led_on(led_green)` 调用一字不差，底下走不同实现 |
| ch12 | 向上转型 | `led_green = (led_base_t *)&_led_green_inst;` |
| ch13 | container_of | `_led_gpio_on` 里 `(led_gpio_t *)me`（base 在第一字段时简化形态） |
| ch14 | 纯虚 | `led_base_ops_t` 里 `led_on / led_off` 是 pure virtual，子类必须填，不填直接崩 |
| ch15 | platform 抽象到底 | `platform_pin_xxx` 封装函数体内走 `_g_ops` dispatch，对外签名一字不变 |
| ch16 | Linux 风格 | `container_of`、`platform_pin_register`、`INIT_xxx_EXPORT` 全是 Linux 内核风 |
| ch17 | 链接初始化 | 8 级 `INIT_xxx_EXPORT` 宏配链接脚本 8 个段 |

每一章的抽象都在这个工程里有对应的代码位置。读到这一附录，你应该能拿着这份代码当作"全书的活样本"。

## B.6 进阶练习

如果你跑通了流水灯，下面几个改造可以验证你对抽象的理解：

**练习 1：** 加一颗 PWM 接口的 LED。新增 `app/drivers/led/led_pwm.c`，定义 `led_pwm_t`，实现 `led_pwm_init / _led_pwm_on / _led_pwm_off`，在 `environment_cfg/led_cfg.c` 里加一行 `led_pwm_init(...)`。**应用层 main.c 一行不改**。这是这本书要教给你的开闭原则。

**练习 2：** 加一个 button 模块。`button_base.h / button_base_ops_t / button_gpio_t` 同样四件套。`environment_cfg/button_cfg.c` 装配实例 + `INIT_DEVICE_EXPORT`。在 main 里检测按下切换 LED 模式（流水灯 vs 全亮 vs 闪烁）。

**练习 3：** 换板。把整个工程移植到 STM32F103（蓝色小板）。除了 `app/platform/arch/board/pin_board.c` 之外，**应用层和 driver 一行不改能编过**。如果改了，停下来想想抽象哪里还不够干净。

## B.7 已知工程性细节

工程结构完整、抽象覆盖全，PC 模拟模式 0 警告 0 错误跑通。下面几点真机移植时按 CubeMX 生成版补全：

- `linker_stm32f407.ld` 中 `_Min_Heap_Size` / `_Min_Stack_Size` / `.preinit_array` / `.init_array` / `.ARM.exidx` 段：CubeMX 生成的链接脚本里有
- `startup_stm32f407xx.s` 的 80+ 中断向量：CubeMX 生成版整段抄进来
- `Core/Src/main.c` 里 `SystemClock_Config` 和 `MX_GPIO_Init`：CubeMX 生成版抄进来
- `pin_board.c` 的 IRQ ops（`pin_attach_irq` 等）：教学版留 NULL，工业版补全

这些都是 CubeMX 自动生成的工程性细节，不影响 OOP 抽象的教学价值。

## 下一篇

附录 C 是 Linux 用户态版本的完整工程，结构和这一附录对称。同样的 OOP 抽象在 ARM64 SBC（树莓派 / 香橙派）的 Linux 用户态跑一遍，看完两个附录你能切身感受到"换平台不改应用"的威力。

下一篇：[附录 C · Linux 用户态完整工程](C-Linux完整工程.md)
