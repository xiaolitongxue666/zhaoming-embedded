# 附录 B · STM32 完整工程：跑通全书所有抽象

参考代码：[`industrial/stm32_full/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/industrial/stm32_full/)。

附录 B 和附录 C 是这本书的"集大成"工程。每章配套的 PC 模拟代码已经把每个概念讲透了，但读者会有一个合理的诉求：**能不能给我一个跑在真实 MCU 上的完整工程，里面所有抽象（struct + me、ops 表、container_of、platform_pin、initcall）一次跑通？**

这两个附录就是给你这个工程。附录 B 是 STM32（Cortex-M 系列），附录 C 是 Linux 用户态（ARM64 SBC）。

> 工程参照真实工业项目（同款 STM32 Cortex-M 控制板）的代码组织方式整理而成。引脚配置 / HAL 参数按 STM32F407 Discovery 这块流通最广的开发板写出，换其他 STM32 系列（H7 / F1 / L4 / G0 等）只改 `arch/board/pin_board.c` 一份文件，应用层和驱动层一行不动。

### 附录 B 在全书的位置

附录 B 是这本书"裸机必须自抽 platform"的实战兑现，跟 ch15 / ch16 章节正文 + ch16 § 16.13 / § 16.14 的工程判断力教学呼应：

- **ch15 教学版**（`oop-in-c/code/15-platform/`）把四层架构（drivers + platform + platform/arch/{stm32,nxp} + linux-driver/userspace）讲清楚，每个抽象一次只讲一件事
- **附录 B 工程**（`industrial/stm32_full/`）把同套抽象按真实工业项目的纪律重组：跨编译器宏（ARMCC / IAR / GCC）、`platform_err_t` 错误码、7 级 initcall + 第 8 级单测段、`platform_assert` 校验、字符串 PIN 名解析、三层信息可见性
- **ch16 § 16.13 / § 16.14**：内核已做完别再抽。Zephyr / RT-Thread / NuttX 的 device subsystem 都是同款。MPU / SoC 直接用 Linux

附录 B 是给你"在没有内核 driver model 帮忙、只能自己抽"的环境里参考的工业级骨架。读完之后你应当能判断：哪些场景值得花这一份力气自抽，哪些场景该 clone Zephyr / RT-Thread 的 driver framework 直接用。这本书 ch15 § 15.16 给的真实工程建议（**MCU 用 Zephyr / RT-Thread, MPU / SoC 用 Linux, 全平台都不要自己抽 platform 层**）是这一附录工程之外要带走的更高一层判断力。

## B.1 工程概览

目标硬件：**STM32F407 Discovery 板**（Cortex-M4F，168 MHz，1 MB Flash，192 KB SRAM）。这是国内最容易拿到的开发板之一，淘宝几十块到一百多块。

工程实现的功能：板上 4 颗 LED 演示 **GPIO + PWM + I²C 三种子类混搭**：

| 句柄 | 子类 | 资源 | set_brightness 行为 |
|---|---|---|---|
| `led_status` | `struct led_gpio` | PD.12, 高电平点亮 | no-op (走父类默认) |
| `led_dimmer` | `struct led_pwm` | PWM ch0 | 真生效, duty 0-255 |
| `led_panel` | `struct led_i2c` | I²C 0x3C, reg 0x00 | no-op |
| `led_alarm` | `struct led_gpio` | PD.15, 高电平点亮 | no-op |

应用层只看到 4 个 `struct led_base *` 句柄，看不到子类完整类型（`struct led_gpio` / `struct led_pwm` / `struct led_i2c`），调用一致：

```c
led_on(led_status);                  /* GPIO 子类底下拉 PD.12 高 */
led_on(led_dimmer);                  /* PWM 子类底下使能通道 + 写 duty */
led_set_brightness(led_dimmer, 128); /* 父类 dispatch 到 PWM 子类生效 */
led_set_brightness(led_status, 128); /* 父类 dispatch, GPIO 子类未实现, no-op */
```

"换硬件不改应用"在三种维度（GPIO / PWM / I²C）同时演示。这是这本书工业级 OOP 最完整的实战。

代码里跑通了全书所有 OOP 抽象：

- ch01 封装：`struct led_base + me` 指针
- ch10 / ch11 多态：`struct led_ops *ops` + vptr，三子类 dispatch 完整图景
- ch12 向上转型：`struct led_gpio { struct led_base base; ... }`，`(struct led_base *)gpio` 直接转
- ch13 container_of：从基类指针拿回子类的私有字段（base 在第一字段时简化为 `(struct led_gpio *)me`）
- ch14 纯虚：`struct led_ops` 里 `on / off` 是 pure virtual，子类必须实现，父类层 assert 守护
- ch15 platform 抽象到底：`platform_pin_xxx()` / `platform_pwm_xxx()` / `platform_i2c_xxx()` 三层 ops 表 + register 机制
- ch16 platform 升级：`pin_board.c` / `pwm_board.c` / `i2c_board.c` 三个板级子类挂上来（PC mock 与真机 STM32 各一份）
- ch17 链接自动初始化：8 级 `INIT_xxx_EXPORT` 宏 + 第 8 级 `UNIT_TEST_EXPORT`

## B.2 工业纪律

附录 B 的工程结构脱胎于真实工业项目（脱敏后保留教学需要的最小集合）。三层信息可见性差别是工业纪律的核心：

| 层 | 看得到的内容 | 看不到的内容 |
|---|---|---|
| **应用层** (`Core/Src/main.c` / `mock/main_pc.c`) | `struct led_base *` 句柄 | 子类完整类型、ops 表、平台细节 |
| **驱动层** (`app/drivers/led/`) | `struct led_gpio` / `led_pwm` / `led_i2c`、`platform_xxx` 封装函数 | `struct platform_xxx_ops`、寄存器、HAL |
| **平台层** (`app/platform/`、`app/platform/arch/board/`) | 所有 ops 表、寄存器、HAL | / |

跨边界的硬规则：

- 驱动层 `#include "platform/platform_pin.h"` / `platform_pwm.h` / `platform_i2c.h` 调封装函数，**永远不**直接碰寄存器或 HAL
- 平台层框架（`platform_pin.c` / `platform_pwm.c` / `platform_i2c.c`）维护 `static const struct platform_xxx_ops *_g_ops` 指针，子类通过 `platform_xxx_register(&xxx_ops)` 启动期填进来
- 字符串 pin 名（`"PA.5"` / `"PD.12"`）：调用方写字面字符串，看不到 port 索引或寄存器地址。换芯片只改 `pin_board.c` 的解码

## B.3 工程结构

```
stm32_full/
├── README.md
├── Makefile                                 # 双模 build (真机 + MOCK)
├── linker_stm32f407.ld                      # 链接脚本 (含 8 个 moduleExport 段)
├── startup_stm32f407xx.s                    # 启动汇编骨架
├── Core/                                    # CubeMX 风格的 MCU 启动入口
│   ├── Inc/main.h
│   └── Src/main.c                           # 真机 main: HAL_Init + ... + module_export_exec
├── app/                                     # 应用层
│   ├── project_config.h                     # PLATFORM_OS / PLATFORM_HEAP_ENABLE 开关
│   ├── platform/                            # 平台抽象层
│   │   ├── platform_def.h                   # 跨编译器宏 + platform_err_t + container_of
│   │   ├── platform_assert.h / .c
│   │   ├── platform_pin.h / .c              # PIN framework (字符串名 + ops 分发)
│   │   ├── platform_pwm.h / .c              # PWM framework (channel + duty 0-255)
│   │   ├── platform_i2c.h / .c              # I2C framework (bus + client 二层 + master_xfer)
│   │   ├── platform_module_export.h / .c    # 8 级 INIT_xxx_EXPORT (ARMCC/IAR/GCC + MOCK)
│   │   └── arch/board/
│   │       ├── pin_board.c                  # STM32 pin 子类  (HAL_GPIO_*)
│   │       ├── pwm_board.c                  # STM32 pwm 子类  (HAL_TIM_PWM_*)
│   │       └── i2c_board.c                  # STM32 i2c bus   (HAL_I2C_Master_*)
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

## B.4 关键文件

> **API 演化说明**：教学章节 ch01-ch15 用的是 `platform_gpio_init/write/read/deinit(uint8_t pin, ...)` 函数式 API（一组独立函数 + 数字 pin 编号），是**简化形态**便于教学引入。
>
> 附录 B 工程演化到工业级形态：
>
> - API 改名为 `platform_pin_init/write/read(int32_t pin, ...)`，跟 RT-Thread / 国内多数控制板项目命名一致
> - **字符串 pin 名**：调用方写 `"PA.5"` / `"PD.12"` / `"PI.14"`，让 platform 层内部解析。换芯片只改解析表，driver 一字不动
> - 错误码用统一的 `platform_err_t` 枚举（`PLATFORM_EOK / PLATFORM_EINVAL / ...`），所有 init 函数返回它
> - 所有数据结构用 **struct 风格**：`struct led_base` / `struct led_ops` / `struct platform_pin_ops`，不再 typedef，跟 Linux 内核风格一致
>
> 这一组演化跟 ch15 / ch16 讲的"对外封装、对内 ops"是一回事，只是从最简函数式逐步推到工业版形态。读者把附录 B 工程跟 ch15 / ch16 的最终形态对比，能直观看出"工业代码长什么样"。

### app/platform/platform_def.h（跨编译器宏 + 错误码 + container_of）

```c
typedef enum {
	PLATFORM_EOK       =  0,    /* 没错 */
	PLATFORM_ERROR     = -1,    /* 通用错误 */
	PLATFORM_ETIMEOUT  = -2,    /* 超时 */
	PLATFORM_EFULL     = -3,    /* 资源已满 */
	PLATFORM_EEMPTY    = -4,    /* 资源已空 */
	PLATFORM_ENOMEM    = -5,    /* 内存不足 */
	PLATFORM_ENOSYS    = -6,    /* 不支持 / 未实现 */
	PLATFORM_EBUSY     = -7,    /* 忙 */
	PLATFORM_EIO       = -8,    /* IO 错误 */
	PLATFORM_EINTR     = -9,    /* 系统调用被中断 */
	PLATFORM_EINVAL    = -10    /* 无效参数 */
} platform_err_t;

#ifndef offsetof
#define offsetof(TYPE, MEMBER)  ((unsigned long) &((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
/* Cast a member of a structure out to the containing structure. */
#define container_of(ptr, type, member)                  \
	({                                                   \
		void *__mptr = (void *)(ptr);                    \
		((type *)(__mptr - offsetof(type, member)));     \
	})
#endif

/* 跨编译器 attribute 宏 */
#if defined(__CC_ARM) || defined(__CLANG_ARM)           /* ARM Compiler 5/6 */
	#define PLATFORM_SECTION(x)         __attribute__((section(x)))
	#define PLATFORM_USED               __attribute__((used))
	/* ... */
#elif defined(__IAR_SYSTEMS_ICC__)                       /* IAR */
	#define PLATFORM_SECTION(x)         @ x
	#define PLATFORM_USED               __root
	/* ... */
#elif defined(__GNUC__)                                  /* GCC */
	#define PLATFORM_SECTION(x)         __attribute__((section(x)))
	#define PLATFORM_USED               __attribute__((used))
	/* ... */
#else
	#error not supported tool chain
#endif
```

`platform_err_t` 是工程通用错误码。所有 init / register 函数返回它，调用方用 `if (PLATFORM_EOK != ret) goto exit;` 集中错误处理。

`container_of` 不是 C 标准里的，是 Linux 内核用 `offsetof` + GCC 语句表达式自己拼出来的宏（ch13 详细讲过）。这里复用同款实现。

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
#define PIN_MODE_INPUT_PULLDOWN       0x03
#define PIN_MODE_OUTPUT_OD            0x04

/* ops 表抽象 (子类填写) */
struct platform_pin_ops {
	void (*mode)(int32_t pin, int32_t mode);
	void (*write)(int32_t pin, int32_t value);
	int32_t (*read)(int32_t pin);
	platform_err_t (*attach_irq)(int32_t pin, uint32_t mode,
	                             void (*hdr)(void *args), void *args);
	platform_err_t (*detach_irq)(int32_t pin);
	platform_err_t (*irq_enable)(int32_t pin, uint32_t enabled);
	int32_t (*get)(const char *name);
};

/* 注册接口 (子类用) */
platform_err_t platform_pin_register(const struct platform_pin_ops *ops);

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
static const struct platform_pin_ops *_g_ops = NULL;

platform_err_t platform_pin_register(const struct platform_pin_ops *ops)
{
	platform_err_t ret = PLATFORM_EINVAL;

	if (NULL == ops) {
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
	platform_assert(_g_ops->mode != NULL);
	_g_ops->mode(pin, mode);
}

void platform_pin_write(int32_t pin, int32_t value)
{
	platform_assert(_g_ops != NULL);
	platform_assert(_g_ops->write != NULL);
	_g_ops->write(pin, value);
}

int32_t platform_pin_get(const char *name)
{
	int32_t ret = -1;

	platform_assert(_g_ops != NULL);
	platform_assert(name != NULL);
	platform_assert(name[0] == 'P');

	if (NULL == _g_ops->get) {
		ret = PLATFORM_ENOSYS;
		goto exit;
	}

	ret = _g_ops->get(name);

exit:
	return ret;
}
```

`_g_ops` 是 platform 层的私有状态（`static`，文件作用域），外部链接不到。这就是 ch15 讲的"对外封装、对内 ops"形态。换硬件只改一个指针，封装函数签名永远不变。

`platform_pwm` 同样模板，一份 `_g_ops` + register + dispatch 函数。`platform_i2c` 升级到 bus + client 二层（一颗 MCU 一路 I2C 控制器底下挂多颗从设备，所以 ops 挂在 bus 实例上而不是全局指针），后面单独讲。三个 framework 子类各自独立挂载。

### app/platform/platform_pwm.h（PWM 框架对外 API）

```c
/* PWM duty 范围: 0-255 (线性, 8 位) */
#define PLATFORM_PWM_DUTY_MIN         0
#define PLATFORM_PWM_DUTY_MAX         255

/* ops 表抽象 (子类填写) */
struct platform_pwm_ops {
	platform_err_t (*enable)(int32_t channel);
	platform_err_t (*disable)(int32_t channel);
	platform_err_t (*set_duty)(int32_t channel, uint8_t duty);
};

/* 注册接口 (子类用) */
platform_err_t platform_pwm_register(const struct platform_pwm_ops *ops);

/* 公共 API (上层调) */
platform_err_t platform_pwm_enable(int32_t channel);
platform_err_t platform_pwm_disable(int32_t channel);
platform_err_t platform_pwm_set_duty(int32_t channel, uint8_t duty);
```

`duty` 是 8 位线性值（0-255），driver / 应用层契约。底下板级实现折算成芯片 TIM 的实际寄存器值。`led_pwm` 子类 `set_brightness(level)` 直接把 level 当 duty 写下去，语义对齐。

### app/platform/platform_i2c.h（I2C 框架对外 API：bus + client 二层）

I2C 跟 PIN / PWM 不一样。一颗 MCU 上一路 I2C 控制器底下挂多颗从设备（IO expander / EEPROM / 传感器各占一个 7-bit 地址），所以 I2C 框架是**两层抽象**：

- `struct platform_i2c_bus_device` 一条总线（一路 I2C 控制器），子类填 `master_xfer` 真发命令
- `struct platform_i2c_client` 挂在某条 bus 上的从设备，携带 7-bit 从机地址

```c
/* I2C msg flags */
#define PLATFORM_I2C_WR          (0x0000)
#define PLATFORM_I2C_RD          (1u << 0)
#define PLATFORM_I2C_ADDR_10BIT  (1u << 2)

/* 一段 I2C transfer. 多段拼起来支持"写 reg 地址 + 重启读 N 字节"组合 */
struct platform_i2c_msg {
	uint16_t  addr;
	uint16_t  flags;
	uint16_t  len;
	uint8_t  *buf;
};

struct platform_i2c_bus_device;

/* bus 子类 ops 表 (i2c_board_<chip>.c 填写) */
struct platform_i2c_bus_device_ops {
	platform_err_t (*master_xfer)(struct platform_i2c_bus_device *bus,
	                              struct platform_i2c_msg *msgs,
	                              uint32_t num);
};

struct platform_i2c_bus_device {
	const struct platform_i2c_bus_device_ops *ops;
};

struct platform_i2c_client {
	struct platform_i2c_bus_device *bus;
	uint16_t                        client_addr;
};

/* 注册接口 (i2c_board_<chip>.c 启动期调) */
platform_err_t platform_i2c_bus_register(struct platform_i2c_bus_device *bus,
                                         const struct platform_i2c_bus_device_ops *ops);

/* 公共 API (上层 driver 调) */
platform_err_t platform_i2c_transfer(struct platform_i2c_bus_device *bus,
                                     struct platform_i2c_msg *msgs,
                                     uint32_t num);
```

跟 `industrial/platform_layer/platform_i2c.h` 工业完整版接口字节对齐，只是教学版砍掉 `osMutex lock` / `platform_device parent` / `slave_xfer / i2c_bus_control / master_send / master_recv` 这些依赖 RTOS 和设备表的部分，保留最常用的 `master_xfer` 主路径。换硬件不改应用：driver 层 `#include "platform/platform_i2c.h"` 拼 `platform_i2c_msg` 调 `platform_i2c_transfer`，看不到 I2C 控制器寄存器或 DMA。

### app/platform/arch/board/pin_board.c（STM32 pin 子类，节选）

子类实现 ops 函数，启动期通过 `INIT_BOARD_EXPORT` 自动注册：

```c
#define PIN_NUM(port, no)        (((((port) & 0xFu) << 4) | ((no) & 0xFu)))
#define PIN_PORT(pin)            ((uint8_t)(((pin) >> 4) & 0xFu))
#define PIN_NO(pin)              ((uint8_t)((pin) & 0xFu))
#define PIN_STPORT(pin)          ((GPIO_TypeDef *)(GPIOA_BASE + (0x400u * PIN_PORT(pin))))
#define PIN_STPIN(pin)           ((uint16_t)(1u << PIN_NO(pin)))

/* "PA.5" / "PD.12" / "PI.14" -> pin number, 或 PLATFORM_EINVAL */
static int32_t _stm32_pin_get(const char *name)
{
	int32_t ret = PLATFORM_EINVAL;
	int     hw_port_num;
	int     hw_pin_num = 0;
	int     i;
	int     name_len;

	name_len = (int)strlen(name);

	if ((name_len < 4) || (name_len >= 6)) {
		goto exit;
	}
	if ((name[0] != 'P') || (name[2] != '.')) {
		goto exit;
	}
	if ((name[1] < 'A') || (name[1] > 'Z')) {
		goto exit;
	}
	hw_port_num = (int)(name[1] - 'A');

	for (i = 3; i < name_len; i++) {
		hw_pin_num *= 10;
		hw_pin_num += name[i] - '0';
	}

	ret = PIN_NUM(hw_port_num, hw_pin_num);

exit:
	return ret;
}

static void _stm32_pin_write(int32_t pin, int32_t value)
{
	if (PIN_PORT(pin) >= __STM32_PORT_MAX) {
		goto exit;
	}
	HAL_GPIO_WritePin(PIN_STPORT(pin), PIN_STPIN(pin),
	                  (GPIO_PinState)value);

exit:
	return;
}

/* ... pin_mode / pin_read 同样基于 HAL_GPIO_xxx 实现 */

static const struct platform_pin_ops _stm32_pin_ops = {
	.mode       = _stm32_pin_mode,
	.write      = _stm32_pin_write,
	.read       = _stm32_pin_read,
	.attach_irq = NULL,    /* 教学版未实现 IRQ, 工业版补 */
	.detach_irq = NULL,
	.irq_enable = NULL,
	.get        = _stm32_pin_get,
};

static void _pin_board_init(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	/* ... 所有 port 时钟使能 (条件编译, 缺哪个 port 跳过) */

	(void)platform_pin_register(&_stm32_pin_ops);
}
INIT_BOARD_EXPORT(_pin_board_init);
```

`PIN_NUM(port, num)` 把 port (A=0, B=1, ...) 高 4 位 + 引脚号 (0-15) 低 4 位编码成单参数 `int32_t`。这一招让 `platform_pin_xxx` 的签名永远只有一个 `pin` 参数，无论底下是单 port 的小芯片还是 12 个 port 的大芯片。

跟 Linux 内核 `gpio_set_value(unsigned int gpio, ...)`、Zephyr `gpio_pin_set_dt(spec, value)` 是同一种工业纪律：**对外是单参数（或句柄），port 信息藏在编码里**。

### app/platform/arch/board/pwm_board.c（STM32 pwm 子类）

跟 `pin_board.c` 同款套路：实现一组 ops 函数，启动期 `INIT_BOARD_EXPORT` 注册。差别在底下走 `HAL_TIM_PWM_*`，channel 0..N-1 解码到 `(htim, TIM_CHANNEL_x)` 二维。

```c
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

struct pwm_chan_map {
	TIM_HandleTypeDef *htim;
	uint32_t           channel;
};

static const struct pwm_chan_map _pwm_chan_table[] = {
	{ &htim3, TIM_CHANNEL_1 },     /* channel 0: TIM3_CH1 */
	{ &htim3, TIM_CHANNEL_2 },     /* channel 1: TIM3_CH2 */
	{ &htim4, TIM_CHANNEL_1 },     /* channel 2: TIM4_CH1 */
	{ &htim4, TIM_CHANNEL_2 },     /* channel 3: TIM4_CH2 */
};

static platform_err_t _stm32_pwm_set_duty(int32_t channel, uint8_t duty)
{
	uint32_t arr;
	uint32_t ccr;

	if ((channel < 0) || (channel >= PWM_CHAN_NUM)) {
		return PLATFORM_EINVAL;
	}

	/* duty 0-255 (driver 契约) -> CCR (TIM 寄存器). ARR 从 CubeMX 写好的
	 * 寄存器读出来按比例算 CCR, 跟 ARR 解耦, 换 TIM 周期不用改 driver. */
	arr = __HAL_TIM_GET_AUTORELOAD(_pwm_chan_table[channel].htim);
	ccr = ((uint32_t)duty * arr) / 255u;

	__HAL_TIM_SET_COMPARE(_pwm_chan_table[channel].htim,
	                      _pwm_chan_table[channel].channel, ccr);
	return PLATFORM_EOK;
}

static const struct platform_pwm_ops _stm32_pwm_ops = {
	.enable   = _stm32_pwm_enable,
	.disable  = _stm32_pwm_disable,
	.set_duty = _stm32_pwm_set_duty,
};

static void _pwm_board_init(void)
{
	(void)platform_pwm_register(&_stm32_pwm_ops);
}
INIT_BOARD_EXPORT(_pwm_board_init);
```

要点：

- `htim3 / htim4` 是 CubeMX 生成的 `TIM_HandleTypeDef`，真机 main 侧 `MX_TIM3_Init / MX_TIM4_Init` 之后填进去，板级文件 `extern` 引用，**不重新声明**
- `_pwm_chan_table` 把"线性 channel 编号"解码到"具体哪个 TIM 哪个通道"，跟 pin name 解析的纪律一致：driver / 应用层不知道是 TIM3 还是 TIM4，换板只改这一处
- `__HAL_TIM_GET_AUTORELOAD` 运行期读 ARR，跟 CubeMX 写的 Period 解耦——把 PWM 周期从 1 kHz 改 10 kHz 不改一行 driver 代码

### app/platform/arch/board/i2c_board.c（STM32 i2c bus 子类）

I2C 是 bus + client 二层抽象（一颗 MCU 一路 I2C 控制器 = 一个 `platform_i2c_bus_device` 实例 + 一份 ops），所以 board 这一份只挂一条 i2c1 总线实例，client（led_panel）由 `led_cfg.c` 装配。

```c
extern I2C_HandleTypeDef hi2c1;

#define STM32_I2C_TIMEOUT_MS     1000u

static platform_err_t _stm32_i2c_master_xfer(struct platform_i2c_bus_device *bus,
                                             struct platform_i2c_msg *msgs,
                                             uint32_t num)
{
	HAL_StatusTypeDef st;
	uint32_t          i;
	uint16_t          hal_addr;

	(void)bus;

	for (i = 0; i < num; i++) {
		/* 7-bit 地址 HAL 要左移一位填 R/W bit (HAL 内部按位写). */
		hal_addr = (uint16_t)(msgs[i].addr << 1);

		if (msgs[i].flags & PLATFORM_I2C_RD) {
			st = HAL_I2C_Master_Receive(&hi2c1, hal_addr,
			                            msgs[i].buf, msgs[i].len,
			                            STM32_I2C_TIMEOUT_MS);
		} else {
			st = HAL_I2C_Master_Transmit(&hi2c1, hal_addr,
			                             msgs[i].buf, msgs[i].len,
			                             STM32_I2C_TIMEOUT_MS);
		}

		if (HAL_OK != st) {
			return (HAL_TIMEOUT == st) ? PLATFORM_ETIMEOUT : PLATFORM_EIO;
		}
	}
	return PLATFORM_EOK;
}

static const struct platform_i2c_bus_device_ops _stm32_i2c_bus_ops = {
	.master_xfer = _stm32_i2c_master_xfer,
};

/* 这条 i2c 总线的实例. 应用层 led_cfg.c 在 !MOCK_PC 分支 extern 它. */
struct platform_i2c_bus_device stm32_i2c1_bus;

static void _i2c_board_init(void)
{
	(void)platform_i2c_bus_register(&stm32_i2c1_bus, &_stm32_i2c_bus_ops);
}
INIT_BOARD_EXPORT(_i2c_board_init);
```

要点：

- `stm32_i2c1_bus` 是**实例本身**（不是指针），应用层 `extern` 这个名字、传 `&stm32_i2c1_bus` 给 `led_i2c_init`，跟 mock 端 `pc_i2c_bus0` 在 `led_cfg.c` 里完全对称
- `master_xfer` 是 ops 表唯一的必填字段，循环把 `platform_i2c_msg` 数组发出去；`HAL_TIMEOUT` 翻译成 `PLATFORM_ETIMEOUT`，其他 HAL 错误统一 `PLATFORM_EIO`
- 多条 I2C 总线（i2c2 / i2c3）：复制这一份改 `hi2cx` + 实例名，每条 bus 一个 `platform_i2c_bus_device` 实例 + `INIT_BOARD_EXPORT`

`led_cfg.c` 适配双模 build：

```c
#ifdef MOCK_PC
extern struct platform_i2c_bus_device pc_i2c_bus0;
#define _led_panel_bus    (&pc_i2c_bus0)
#else
extern struct platform_i2c_bus_device stm32_i2c1_bus;
#define _led_panel_bus    (&stm32_i2c1_bus)
#endif

led_i2c_init(&_led_panel_inst, "panel", _led_panel_bus, 0x3C, 0x00);
```

应用层 / driver 一字不动，只在配置层一处分模式选 bus 实例。

### app/platform/platform_module_export.h（8 级 initcall 宏）

```c
struct module_export {
	void (*func)(void);
};

#ifdef MOCK_PC
/* PC mock: GCC ctor 在 main 之前自动跑 */
#define _PLATFORM_INIT_CTOR(_func, _prio)                            \
	__attribute__((constructor(_prio)))                              \
	static void _func##_module_init_ctor(void) { _func(); }

#define INIT_BOARD_EXPORT(fn)            _PLATFORM_INIT_CTOR(fn, 101)
#define INIT_PREV_EXPORT(fn)             _PLATFORM_INIT_CTOR(fn, 102)
#define INIT_DEVICE_EXPORT(fn)           _PLATFORM_INIT_CTOR(fn, 103)
#define INIT_COMPONENT_EXPORT(fn)        _PLATFORM_INIT_CTOR(fn, 104)
#define INIT_ENV_EXPORT(fn)              _PLATFORM_INIT_CTOR(fn, 105)
#define INIT_APP_EXPORT(fn)              _PLATFORM_INIT_CTOR(fn, 106)
#define INIT_SYSTEM_READY_EXPORT(fn)     _PLATFORM_INIT_CTOR(fn, 107)
#define UNIT_TEST_EXPORT(fn)             _PLATFORM_INIT_CTOR(fn, 108)

#else  /* ! MOCK_PC: 真机三大编译器 */

#define INIT_EXPORT(_func, level)                                   \
	MODULE_EXPORT_USED const struct module_export                   \
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

7 个优先级覆盖一个工业项目从板级到应用层的所有启动顺序：

```
1 BOARD       板级 (时钟 / GPIO clock)
2 PREV        软件预初始化 (heap / sys)
3 DEVICE      设备驱动 (uart / i2c / spi / pin)
4 COMPONENT   组件 (log / shell / FS)
5 ENV         环境配置 (LED 实例配置 / 设备绑定)
6 APP         应用任务
7 SYSTEM_READY 系统就绪
8 UNIT_TEST   单元测试 (跟前 7 级独立)
```

启动时 `platform_module_export_exec()` 顺序跑 1-7 级。第 8 级 `UNIT_TEST_EXPORT` 单独由 `platform_unit_test_exec()` 跑——平时不调，只在跑测试镜像时调。

跨编译器（ARMCC `$$Base/$$Limit`、IAR `__section_begin`、GCC `_start/_end`）通过条件编译统一接口。MOCK 模式用 `__attribute__((constructor(N)))` 直接让 GCC 在 main 之前自动跑，**应用层调 `platform_module_export_exec()` 在两种模式下调用形态完全一致**。

### app/drivers/led/led_base.h（基类）

```c
struct led_base;

/* led_ops - LED 子类必须实现的虚函数表.
 *
 *   on / off                纯虚, 子类必填, 父类 dispatch 时 assert
 *   set_brightness          可选, 子类不填走父类默认 no-op
 */
struct led_ops {
	platform_err_t (*on)(struct led_base *me);
	platform_err_t (*off)(struct led_base *me);
	platform_err_t (*set_brightness)(struct led_base *me, uint8_t level);
};

/* led_base - 所有 LED 子类的父类.
 *
 *   ops    指向子类的虚函数表 (static const, 实例间共享)
 *   name   实例名, 调试 / 日志友好
 *   is_on  当前开关状态, 父类记录, 子类不要直接改
 */
struct led_base {
	const struct led_ops *ops;
	const char           *name;
	bool                  is_on;
};

/* 公开接口 - 应用层只调这一组, 不直接访问 ops */
platform_err_t led_base_init(struct led_base *me, const char *name,
                             const struct led_ops *ops);
platform_err_t led_on(struct led_base *me);
platform_err_t led_off(struct led_base *me);
platform_err_t led_set_brightness(struct led_base *me, uint8_t level);
```

要点：

- `struct led_base` / `struct led_ops` 都是 **struct 风格**，没有 typedef。这是 Linux 内核风格，附录 B 工程统一这一种。
- `ops` 是 `const struct led_ops *`，子类 ops 表是 `static const`（在 Flash 里），所有实例共享同一张表（ch10 vtable 内存布局）。
- `name` / `is_on` 是父类层维护的通用字段，子类不要直接改 `is_on`，让父类 dispatch 完毕后统一更新。
- `set_brightness` 是**选填**纯虚（子类不填走父类默认 no-op），on / off 是**必填**纯虚（不填父类 assert 报错）。

### app/drivers/led/led_base.c（dispatch + 必填守护）

```c
platform_err_t led_base_init(struct led_base *me, const char *name,
                             const struct led_ops *ops)
{
	platform_err_t ret = PLATFORM_EOK;

	if ((NULL == me) || (NULL == name) || (NULL == ops)) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	me->ops   = ops;
	me->name  = name;
	me->is_on = false;

exit:
	return ret;
}

platform_err_t led_on(struct led_base *me)
{
	platform_err_t ret;

	if (NULL == me) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	platform_assert(me->ops != NULL);
	platform_assert(me->ops->on != NULL);   /* 纯虚必填 */

	ret = me->ops->on(me);
	if (PLATFORM_EOK == ret) {
		me->is_on = true;
	}

exit:
	return ret;
}

platform_err_t led_set_brightness(struct led_base *me, uint8_t level)
{
	platform_err_t ret;

	if (NULL == me) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	platform_assert(me->ops != NULL);

	if (NULL == me->ops->set_brightness) {
		/* 选填: 父类默认 no-op (GPIO / I2C 简单 LED 不支持调亮度) */
		ret = PLATFORM_EOK;
		goto exit;
	}

	ret = me->ops->set_brightness(me, level);

exit:
	return ret;
}
```

`led_on` / `led_off` / `led_set_brightness` 是**基类层封装函数**，函数体只做三件事：

1. 入参合法性检查（`me != NULL`）
2. assert 守护必填的 ops 字段（让子类忘填纯虚函数时第一时间崩在这里，而不是在子类里乱跳）
3. dispatch 到子类 ops，dispatch 成功后统一更新父类层 `is_on` 状态

ch11 讲的"基类层包装函数"形态在这里完整落地：必填用 `assert`，选填用 `if (NULL == ...) 走默认`。

### app/drivers/led/led_gpio.h / .c（GPIO 子类）

```c
/* led_gpio.h */
struct led_gpio {
	struct led_base base;        /* 基类放第一字段, 上转直接 cast */
	int32_t         pin;         /* platform_pin_get 解析后的 pin 号 */
	bool            active_high; /* true: 高电平点亮 */
};

/* 构造函数.
 *   me           子类实例
 *   name         实例名 (如 "status", "alarm")
 *   pin_name     平台 pin 名 (如 "PD.12")
 *   active_high  true 表示高电平点亮
 */
platform_err_t led_gpio_init(struct led_gpio *me, const char *name,
                             const char *pin_name, bool active_high);
```

```c
/* led_gpio.c */
#include "drivers/led/led_gpio.h"
#include "platform/platform_pin.h"

static platform_err_t _led_gpio_on(struct led_base *me);
static platform_err_t _led_gpio_off(struct led_base *me);

/* static const ops 表给所有 led_gpio 实例共享, 放 Flash. */
static const struct led_ops led_gpio_ops = {
	.on             = _led_gpio_on,
	.off            = _led_gpio_off,
	.set_brightness = NULL,    /* GPIO LED 不支持调亮度, 走父类默认 no-op */
};

platform_err_t led_gpio_init(struct led_gpio *me, const char *name,
                             const char *pin_name, bool active_high)
{
	platform_err_t ret;
	int32_t        pin;

	if ((NULL == me) || (NULL == pin_name)) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	pin = platform_pin_get(pin_name);
	if (pin < 0) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	platform_pin_mode(pin, PIN_MODE_OUTPUT);
	platform_pin_write(pin, active_high ? PIN_LOW : PIN_HIGH);

	me->pin         = pin;
	me->active_high = active_high;

	ret = led_base_init(&me->base, name, &led_gpio_ops);

exit:
	return ret;
}

static platform_err_t _led_gpio_on(struct led_base *me)
{
	struct led_gpio *gpio = (struct led_gpio *)me;

	platform_pin_write(gpio->pin, gpio->active_high ? PIN_HIGH : PIN_LOW);
	return PLATFORM_EOK;
}

static platform_err_t _led_gpio_off(struct led_base *me)
{
	struct led_gpio *gpio = (struct led_gpio *)me;

	platform_pin_write(gpio->pin, gpio->active_high ? PIN_LOW : PIN_HIGH);
	return PLATFORM_EOK;
}
```

子类的 ops 表 `led_gpio_ops` 是 `static const`，所有 `struct led_gpio` 实例共享同一张表（在 Flash 里）。`(struct led_gpio *)me` 直接把 base 指针转回子类，因为 `base` 在第一字段。等价的 Linux 内核风格写法：

```c
static platform_err_t _led_gpio_on(struct led_base *me)
{
	struct led_gpio *gpio = container_of(me, struct led_gpio, base);
	/* ... */
}
```

两种写法机器码一字不差。`base` 在第一字段时偏移 0，`container_of` 减去 0 等于直接 cast。

### app/drivers/led/led_pwm.h / .c（PWM 子类，实现 set_brightness）

```c
/* led_pwm.h */
struct led_pwm {
	struct led_base base;
	int32_t         channel;     /* PWM 通道 */
	uint8_t         brightness;  /* 当前亮度 0-255, on 时写入 */
};

platform_err_t led_pwm_init(struct led_pwm *me, const char *name,
                            int32_t channel);
```

```c
/* led_pwm.c */
#include "drivers/led/led_pwm.h"
#include "platform/platform_pwm.h"

#define LED_PWM_DEFAULT_BRIGHTNESS    255

static platform_err_t _led_pwm_on(struct led_base *me);
static platform_err_t _led_pwm_off(struct led_base *me);
static platform_err_t _led_pwm_set_brightness(
	struct led_base *me, uint8_t level);

static const struct led_ops led_pwm_ops = {
	.on             = _led_pwm_on,
	.off            = _led_pwm_off,
	.set_brightness = _led_pwm_set_brightness,
};

platform_err_t led_pwm_init(struct led_pwm *me, const char *name,
                            int32_t channel)
{
	platform_err_t ret;

	if (NULL == me) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	me->channel    = channel;
	me->brightness = LED_PWM_DEFAULT_BRIGHTNESS;

	(void)platform_pwm_disable(channel);
	(void)platform_pwm_set_duty(channel, 0);

	ret = led_base_init(&me->base, name, &led_pwm_ops);

exit:
	return ret;
}

static platform_err_t _led_pwm_on(struct led_base *me)
{
	struct led_pwm *pwm = (struct led_pwm *)me;
	platform_err_t  ret;

	ret = platform_pwm_enable(pwm->channel);
	if (PLATFORM_EOK != ret) {
		goto exit;
	}

	ret = platform_pwm_set_duty(pwm->channel, pwm->brightness);

exit:
	return ret;
}

static platform_err_t _led_pwm_set_brightness(
	struct led_base *me, uint8_t level)
{
	struct led_pwm *pwm = (struct led_pwm *)me;
	platform_err_t  ret = PLATFORM_EOK;

	pwm->brightness = level;

	/* 如果当前是亮的, 立刻把新亮度推下去, 否则只更新缓存值 */
	if (me->is_on) {
		ret = platform_pwm_set_duty(pwm->channel, level);
	}

	return ret;
}
```

`led_pwm` 和 `led_gpio` 的关键差别：**ops 表第三字段填了 `_led_pwm_set_brightness` 真实现**。父类 `led_set_brightness` dispatch 到这里时真生效；GPIO / I2C 子类填 NULL，父类走默认 no-op。

应用层调 `led_set_brightness(led_dimmer, 128)` 真调亮度，调 `led_set_brightness(led_status, 128)` 是空操作——但调用方代码一字不差。这就是"接口一致、行为按子类决定"的多态完整图景。

`set_brightness` 的内部状态机：上电缓存 brightness=255，`led_on` 时把 brightness 推下去；`set_brightness` 时如果当前 `is_on=true` 立刻推新 duty，否则只更新缓存值，等下次 `led_on` 才生效。这是工业代码常见的"状态变更不一定立即生效"模式。

### app/drivers/led/led_i2c.h / .c（I²C 子类，client 模式）

`led_i2c` 是 I2C bus 上挂的一颗 client，把 `struct platform_i2c_client` 嵌入子类，构造时传 bus 句柄 + 7-bit 从机地址进来。on / off 拼一段 `platform_i2c_msg`（reg 地址 + value）走 `platform_i2c_transfer`。跟 EEPROM / 传感器 client 走的是同一套接口。

```c
/* led_i2c.h */
struct led_i2c {
	struct led_base             base;
	struct platform_i2c_client  client;    /* 嵌入式 client (bus + 从机地址) */
	uint8_t                     reg;
	uint8_t                     val_on;
	uint8_t                     val_off;
};

platform_err_t led_i2c_init(struct led_i2c *me, const char *name,
                            struct platform_i2c_bus_device *bus,
                            uint16_t client_addr, uint8_t reg);
```

```c
/* led_i2c.c */
#include "drivers/led/led_i2c.h"
#include "platform/platform_i2c.h"

#define LED_I2C_DEFAULT_VAL_ON     0x01
#define LED_I2C_DEFAULT_VAL_OFF    0x00

static platform_err_t _led_i2c_on(struct led_base *me);
static platform_err_t _led_i2c_off(struct led_base *me);
static platform_err_t _led_i2c_write_reg(struct led_i2c *i2c, uint8_t value);

static const struct led_ops led_i2c_ops = {
	.on             = _led_i2c_on,
	.off            = _led_i2c_off,
	.set_brightness = NULL,    /* I2C LED 不支持调亮度, 走父类默认 no-op */
};

platform_err_t led_i2c_init(struct led_i2c *me, const char *name,
                            struct platform_i2c_bus_device *bus,
                            uint16_t client_addr, uint8_t reg)
{
	platform_err_t ret;

	if ((NULL == me) || (NULL == bus)) {
		ret = PLATFORM_EINVAL;
		goto exit;
	}

	me->client.bus         = bus;
	me->client.client_addr = client_addr;
	me->reg                = reg;
	me->val_on             = LED_I2C_DEFAULT_VAL_ON;
	me->val_off            = LED_I2C_DEFAULT_VAL_OFF;

	ret = led_base_init(&me->base, name, &led_i2c_ops);

exit:
	return ret;
}

static platform_err_t _led_i2c_on(struct led_base *me)
{
	struct led_i2c *i2c = (struct led_i2c *)me;
	return _led_i2c_write_reg(i2c, i2c->val_on);
}

static platform_err_t _led_i2c_off(struct led_base *me)
{
	struct led_i2c *i2c = (struct led_i2c *)me;
	return _led_i2c_write_reg(i2c, i2c->val_off);
}

/* 拼 1 段 msg (2 字节: reg 地址 + value), 走 platform_i2c_transfer 发出去.
 * 等价于 i2c_smbus_write_byte_data(client_addr, reg, value). */
static platform_err_t _led_i2c_write_reg(struct led_i2c *i2c, uint8_t value)
{
	uint8_t                  buf[2];
	struct platform_i2c_msg  msg;

	buf[0] = i2c->reg;
	buf[1] = value;

	msg.addr  = i2c->client.client_addr;
	msg.flags = PLATFORM_I2C_WR;
	msg.len   = 2;
	msg.buf   = buf;

	return platform_i2c_transfer(i2c->client.bus, &msg, 1);
}
```

`val_on` / `val_off` 默认 `0x01 / 0x00`，适配大多数 IO expander 风格的 LED 控制器（PCA9554 / TCA6408 之类）。不同芯片可在子类构造之后再覆盖。

`led_i2c` 和 `led_gpio` 在应用层视角下完全等价：`led_on(led_panel)` 跟 `led_on(led_status)` 调用一字不差，底下一个走 GPIO 寄存器、一个走 I²C 总线。**换硬件不改应用**在这里是"换总线"维度的演示。

### app/environment_cfg/environment_export.h（4 颗 LED 句柄统一导出）

```c
#include "drivers/led/led_base.h"

/* 4 颗 LED, 演示 3 种子类混搭:
 *   led_status   GPIO LED   板上状态指示, 高电平点亮
 *   led_dimmer   PWM  LED   亮度可调, 通道 0
 *   led_panel    I2C  LED   面板 LED 控制器, 0x3C 寄存器 0x00
 *   led_alarm    GPIO LED   告警, 高电平点亮
 */
extern struct led_base *led_status;
extern struct led_base *led_dimmer;
extern struct led_base *led_panel;
extern struct led_base *led_alarm;
```

应用层只 `#include "environment_cfg/environment_export.h"` 一次，拿到全部 4 个句柄。子类完整类型（`struct led_gpio` / `struct led_pwm` / `struct led_i2c`）应用层看不到，只看到 `struct led_base *` 接口。

### app/environment_cfg/led_cfg.c（4 颗 LED 实例装配 + INIT_ENV_EXPORT）

```c
#include "drivers/led/led_gpio.h"
#include "drivers/led/led_i2c.h"
#include "drivers/led/led_pwm.h"
#include "environment_cfg/environment_export.h"
#include "platform/platform_i2c.h"
#include "platform/platform_module_export.h"

/* 板级注册的 I2C bus 句柄. 双模 build 各自指向自己的实例:
 *   MOCK_PC          -> mock/i2c_board_pc.c   定义的 pc_i2c_bus0
 *   非 MOCK_PC (真机) -> arch/board/i2c_board.c 定义的 stm32_i2c1_bus
 * 应用层只看到 _led_panel_bus 一个名字, 换板换 MCU 不用动. */
#ifdef MOCK_PC
extern struct platform_i2c_bus_device pc_i2c_bus0;
#define _led_panel_bus    (&pc_i2c_bus0)
#else
extern struct platform_i2c_bus_device stm32_i2c1_bus;
#define _led_panel_bus    (&stm32_i2c1_bus)
#endif

/* 应用层句柄: 4 颗 LED, 全部 struct led_base * */
struct led_base *led_status = NULL;
struct led_base *led_dimmer = NULL;
struct led_base *led_panel  = NULL;
struct led_base *led_alarm  = NULL;

/* 静态实例池: 子类完整类型, static 让应用层看不到 */
static struct led_gpio _led_status_inst;
static struct led_pwm  _led_dimmer_inst;
static struct led_i2c  _led_panel_inst;
static struct led_gpio _led_alarm_inst;

static void _led_cfg(void)
{
	platform_err_t ret;

	ret = led_gpio_init(&_led_status_inst, "status", "PD.12", true);
	if (PLATFORM_EOK != ret) {
		printf("[led_cfg] status init failed: %d\n", (int)ret);
		goto exit;
	}
	led_status = &_led_status_inst.base;

	ret = led_pwm_init(&_led_dimmer_inst, "dimmer", 0);
	if (PLATFORM_EOK != ret) {
		printf("[led_cfg] dimmer init failed: %d\n", (int)ret);
		goto exit;
	}
	led_dimmer = &_led_dimmer_inst.base;

	ret = led_i2c_init(&_led_panel_inst, "panel", _led_panel_bus, 0x3C, 0x00);
	if (PLATFORM_EOK != ret) {
		printf("[led_cfg] panel init failed: %d\n", (int)ret);
		goto exit;
	}
	led_panel = &_led_panel_inst.base;

	ret = led_gpio_init(&_led_alarm_inst, "alarm", "PD.15", true);
	if (PLATFORM_EOK != ret) {
		printf("[led_cfg] alarm init failed: %d\n", (int)ret);
		goto exit;
	}
	led_alarm = &_led_alarm_inst.base;

exit:
	return;
}
INIT_ENV_EXPORT(_led_cfg);
```

注意：

- 4 颗 LED 全是 `struct led_base *` 类型对外暴露，应用层只看到接口
- 静态实例 `_led_xxx_inst` 用 `static` 修饰，文件外链接不到（数量固定的全局对象走静态实例，跟 ch04 数据归位选择一致）
- 启动时 `INIT_ENV_EXPORT(_led_cfg)` 自动调用 `_led_cfg`，不用手动写在 main 里
- 改板子只改这一份的 4 行实例装配，**应用层 main.c 一行不改**
- 把 `led_pwm_init` 替成 `led_i2c_init`（或反过来），换硬件方案，driver / 应用层一字不动

这就是这本书要教给你的开闭原则（Open/Closed Principle）：对扩展开放、对修改关闭。

### Core/Src/main.c（真机入口，CubeMX 风格）

```c
#include "main.h"

#include "drivers/led/led_base.h"
#include "environment_cfg/environment_export.h"
#include "platform/platform_module_export.h"

/* CubeMX 生成的函数原型 (用户自己 CubeMX 工程提供) */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/* Powered-on busy delay, ~1ms scale @168 MHz */
static void delay_ms(uint32_t ms)
{
	for (volatile uint32_t i = 0; i < ms * 16800UL; i++) {
		;
	}
}

int main(void)
{
	struct led_base *all[4];
	uint32_t         cur = 0;
	uint8_t          dimmer_level = 0;

	/* MCU Configuration */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();

	/* 这一行调用所有 INIT_BOARD/PREV/DEVICE/COMPONENT/ENV/APP/SYSTEM_READY
	 * 注册项: pin_board.c 注册 pin ops, led_cfg.c 装配 4 颗 LED 实例. */
	platform_module_export_exec();

	all[0] = led_status;
	all[1] = led_dimmer;
	all[2] = led_panel;
	all[3] = led_alarm;

	while (1) {
		/* 应用层: 流水灯, 同时演示 dimmer 调亮度 */
		for (uint32_t i = 0; i < 4; i++) {
			(void)led_off(all[i]);
		}
		(void)led_on(all[cur]);
		(void)led_set_brightness(all[cur], dimmer_level);

		cur = (cur + 1) % 4;
		dimmer_level += 32;     /* 0, 32, 64, 96, ... 自然回卷 */
		delay_ms(1000);
	}
}
```

`main` 一共 30 行不到。所有"哪里初始化 LED"、"GPIO 怎么配"、"用哪个 port"、"PWM 怎么调"的复杂度都被抽象层吸收。

### mock/main_pc.c（PC mock 入口）

```c
#include "drivers/led/led_base.h"
#include "environment_cfg/environment_export.h"
#include "platform/platform_module_export.h"

#define ROUNDS    3

int main(void)
{
	struct led_base *all[4];
	uint32_t         round;
	uint32_t         i;

	printf("=========================================\n");
	printf("  stm32_full PC mock: 4 LED demo\n");
	printf("  GPIO + PWM + I2C 三种子类混搭\n");
	printf("=========================================\n");

	/* 真机这一行跑 7 级 INIT_xxx_EXPORT;
	 * MOCK 下 ctor 已经在 main 之前跑过, 这里是 nop. */
	platform_module_export_exec();

	if ((NULL == led_status) || (NULL == led_dimmer) ||
	    (NULL == led_panel)  || (NULL == led_alarm)) {
		printf("[main] LED handles not ready, exit.\n");
		return -1;
	}

	all[0] = led_status;
	all[1] = led_dimmer;
	all[2] = led_panel;
	all[3] = led_alarm;

	for (round = 0; round < ROUNDS; round++) {
		printf("\n--- round %u ---\n", (unsigned)round);
		for (i = 0; i < 4; i++) {
			printf("[app] led_on(%s)\n", all[i]->name);
			(void)led_on(all[i]);
			(void)led_set_brightness(all[i], 128);
			printf("[app] led_off(%s)\n", all[i]->name);
			(void)led_off(all[i]);
		}
	}

	printf("\n=========================================\n");
	printf("  done (%u rounds)\n", (unsigned)ROUNDS);
	printf("=========================================\n");
	return 0;
}
```

`main_pc.c` 跟 `Core/Src/main.c` 共用同一份 driver / 应用代码，差别只在入口骨架——真机要先跑 `HAL_Init / SystemClock_Config / MX_GPIO_Init`，PC 不需要。

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

## B.5 编译运行步骤

### PC 模拟模式（不依赖 STM32 HAL）

直接验证整套抽象的业务逻辑，所有 GPIO / PWM / I²C 操作走 mock 板级实现打到 stdout：

```bash
cd industrial/stm32_full
make MOCK=1
./build/firmware-pc.exe       # Windows
./build/firmware-pc           # Linux/macOS
```

实测输出（前 40 行）：

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
[app] led_on(alarm)
    [PIN] PD.15 <- HIGH
[app] led_off(alarm)
    [PIN] PD.15 <- LOW

--- round 1 ---
[app] led_on(status)
    [PIN] PD.12 <- HIGH
[app] led_off(status)
    [PIN] PD.12 <- LOW
[app] led_on(dimmer)
    [PWM] ch0 enable
```

3 圈演示后退出，0 警告 0 错误。MOCK 模式下：

- `INIT_BOARD_EXPORT(_pin_board_init)` / `pwm_board_init` / `i2c_board_init` 通过 `__attribute__((constructor(101)))` 在 main 之前自动跑，注册三个 mock ops 表
- `INIT_ENV_EXPORT(_led_cfg)` 通过 `constructor(105)` 自动跑，装配 4 颗 LED 实例
- main 调 `platform_module_export_exec()` 是 nop（ctor 已经跑过）
- 调用形态、应用层代码、驱动层代码跟真机模式**完全一致**

注意输出里 `dimmer` 一次 `led_on` 打两行 PWM duty：第一行是 `_led_pwm_on` 把缓存的 `brightness=255` 推下去，第二行是 `led_set_brightness(128)` 因为 `is_on=true` 立刻把新 duty 推下去。这是 `set_brightness` 状态机的真实演示。

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

如果你的板子 LED 引脚不一样：改 `app/environment_cfg/led_cfg.c` 里 4 行实例装配的字符串。其他文件一字不改。

## B.6 这个工程跑通了什么

按全书 17 章过一遍：

| 章节 | 抽象 | 在这个工程里的位置 |
|---|---|---|
| ch01 | struct + me | 所有 LED 子类 ops 函数第一参数 `struct led_base *me` |
| ch02 | 信息隐藏 | 子类完整类型只在 driver 内部见，应用层只看到 `struct led_base *` |
| ch03 | class 化 | `led_gpio_init / led_pwm_init / led_i2c_init` + `led_on/off/set_brightness` 命名前缀统一 |
| ch04 | 数据归位 | `_led_xxx_inst` 静态实例（板上 4 颗数量固定） |
| ch05 | HAL 漫游 | `pin_board.c` 里 `HAL_GPIO_WritePin / HAL_GPIO_Init` 调用 |
| ch06 | 继承痛点 | `struct led_base.ops` 字段被三种子类（GPIO / PWM / I2C）共享 |
| ch07 | 函数指针 | `struct led_ops` 里的 `platform_err_t (*on)(struct led_base *)` |
| ch08 | 函数指针传参 | `platform_pin_register(&_stm32_pin_ops)` 把整张 ops 表当参数传 |
| ch09 | ops 表雏形 | `struct platform_pin_ops` / `platform_pwm_ops` / `platform_i2c_bus_device_ops` |
| ch10 | ops 放进对象 | `struct led_base.ops` 字段，vtable 在 Flash、vptr 在 RAM |
| ch11 | 多态完整 | `led_on(led_status)` / `led_on(led_dimmer)` / `led_on(led_panel)` 调用一字不差，三种子类底下走不同实现 |
| ch12 | 向上转型 | `led_status = &_led_status_inst.base;` / `led_dimmer = &_led_dimmer_inst.base;` 等 |
| ch13 | container_of | `_led_gpio_on` / `_led_pwm_on` / `_led_i2c_on` 里 `(struct led_xxx *)me`（base 在第一字段时简化形态） |
| ch14 | 纯虚 | `struct led_ops` 里 `on / off` 必填（父类 assert 守护），`set_brightness` 选填（父类默认 no-op） |
| ch15 | platform 抽象到底 | `platform_pin / pwm / i2c` 三层 framework，每层一份 `_g_ops` + dispatch + register |
| ch16 | platform 升级到 ops 表 | `pin_board.c` / `pwm_board.c` / `i2c_board.c` 三个子类启动期通过 `INIT_BOARD_EXPORT` 各自注册 `_stm32_pin_ops` / `_stm32_pwm_ops` / `_stm32_i2c_bus_ops`，driver 一字不动 |
| ch17 | 链接初始化 | 8 级 `INIT_xxx_EXPORT` 宏 + 8 个 `moduleExport` 段，第 8 级 `UNIT_TEST_EXPORT` 单独跑 |

每一章的抽象都在这个工程里有对应的代码位置。读到这一附录，你应该能拿着这份代码当作"全书的活样本"。

> **附录 B 是裸机"必须自抽 platform"的实战，附录 C 是 Linux"内核做完别再抽"的对照实战。两个工程合起来读，才能建立完整工程判断力——什么时候自抽、什么时候别抽。详见 ch15 § 15.15 / § 15.16 + ch16 § 16.13 / § 16.14**

## B.7 进阶练习

如果你跑通了流水灯，下面几个改造可以验证你对抽象的理解：

**练习 1：** 加一个 button 模块。`button_base.h / struct button_ops / struct button_gpio` 同样四件套。`environment_cfg/button_cfg.c` 装配实例 + `INIT_DEVICE_EXPORT`。在 main 里检测按下切换 LED 模式（流水灯 vs 全亮 vs 闪烁）。

**练习 2：** 把 `led_dimmer` 的子类从 `led_pwm` 换成 `led_i2c`（假设你有一颗 I²C 控制的可调亮度 LED 驱动器，比如 LP5562）。在 `led_i2c` 子类里实现 `set_brightness`（写控制器的亮度寄存器），`environment_cfg/led_cfg.c` 把 `led_pwm_init(&_led_dimmer_inst, ...)` 替成 `led_i2c_init(&_led_dimmer_inst, ...)`，**应用层 main.c 一行不改**。

**练习 3：** 换板。把整个工程移植到 STM32F103（蓝色小板）。除了 `app/platform/arch/board/pin_board.c` 之外，**应用层和 driver 一行不改能编过**。如果改了，停下来想想抽象哪里还不够干净。

## B.8 已知工程性细节

工程结构完整、抽象覆盖全，PC 模拟模式 0 警告 0 错误跑通。下面几点真机移植时按 CubeMX 生成版补全：

- `linker_stm32f407.ld` 中 `_Min_Heap_Size` / `_Min_Stack_Size` / `.preinit_array` / `.init_array` / `.ARM.exidx` 段：CubeMX 生成的链接脚本里有
- `startup_stm32f407xx.s` 的 80+ 中断向量：CubeMX 生成版整段抄进来
- `Core/Src/main.c` 里 `SystemClock_Config` / `MX_GPIO_Init` / `MX_TIM3_Init` / `MX_TIM4_Init` / `MX_I2C1_Init`：用 CubeMX 把 TIM3 / TIM4 配 PWM 输出（4 个通道接到板上 PWM LED），I2C1 配 master，把生成版抄进来
- `pin_board.c` 的 IRQ ops（`pin_attach_irq` 等）：教学版留 NULL，工业版补全
- 真机三个板级文件 (`pin_board.c` / `pwm_board.c` / `i2c_board.c`) 已经齐备，PC mock 三个 (`mock/pin_board_pc.c` / `pwm_board_pc.c` / `i2c_board_pc.c`) 一一对应，双模 build 接口完全一致

这些都是 CubeMX 自动生成的工程性细节，不影响 OOP 抽象的教学价值。

## 下一篇

附录 C 是 Linux 用户态版本的完整工程，结构和这一附录对称——但走的是相反的工程判断：附录 B 是"裸机必须自抽 platform"，附录 C 是"Linux 内核做完别再抽"。两个工程合起来读，才能切身感受到什么时候该抽、什么时候别抽。

下一篇：[附录 C · Linux 用户态完整工程](C-Linux完整工程.md)
