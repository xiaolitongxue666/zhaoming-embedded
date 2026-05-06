# 第 15 章 · 换硬件不改应用 · Platform 抽象到底

配套代码：[`oop-in-c/code/15-platform/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/oop-in-c/code/15-platform/)

封装、继承、多态、向上转型、向下转型、纯虚 / 选填 / 接口，C 里做 OOP 的全部武器你都见过了。

这一章不引入任何新概念。把武器全部组装起来，演示一套完整的 LED 框架。同一份应用代码，运行时切换 PC、STM32、Linux 三种 platform，零修改。

这是 platform 层 ops 化的高潮章。

## 15.1 四层架构

打开配套代码 `oop-in-c/code/15-platform/pc/` 你会看到这些文件：

```
pc/
├── platform_ops.h, platform_ops_pc.c
├── platform_ops_stm32_mock.c
├── platform_ops_linux_mock.c
├── led.h, led.c
├── leds.h, board_init.c
├── app.h, app.c
├── main.c
└── Makefile
```

按调用方向从上往下分四层：

```
应用层      app.c          alarm_blink / status_breathe / power_on_test
LED 驱动层  led.c          led_base + led_gpio / led_pwm / led_i2c
板级层      board_init.c   实例化、绑定全局句柄
平台层      platform_*.c   GPIO / I2C 物理操作
```

每一层只调用下一层。每一层只关心自己。

![文件结构](../assets/ch15/slide1_文件结构.png)

## 15.2 父类层：led_base + 必填选填

`led.h` / `led.c` 是父类层：

```c
struct led_ops {
	int (*on)(struct led_base *me);
	int (*off)(struct led_base *me);
	int (*set_brightness)(struct led_base *me, uint8_t brightness);
};

struct led_base {
	const struct led_ops *ops;
	const char           *name;
	bool                  is_on;
};
```

统一接口分两种处理：

```c
int led_on(struct led_base *me)
{
	if (!me)
		return -1;
	/* 必填：on 是 LED 的核心能力，子类必须实现 */
	assert(me->ops && me->ops->on &&
	       "led_on: subclass must implement on()");
	return me->ops->on(me);
}

int led_set_brightness(struct led_base *me, uint8_t b)
{
	if (!me || !me->ops)
		return -1;
	if (!me->ops->set_brightness) {     /* 选填，默认跳过 */
		printf("  [%s] no dimming, skip\n", me->name);
		return 0;
	}
	return me->ops->set_brightness(me, b);
}
```

`on / off` 是必填，子类不实现就 assert 报错。`set_brightness` 是选填，子类不实现就走默认行为。

![父类代码](../assets/ch15/slide2_父类代码.png)

## 15.3 子类层：三种硬件

子类把 base 嵌在第一个字段（或任意位置，配套代码 ch13 已经证明 container_of 不挑位置），后面追加自己的字段：

```c
struct led_gpio {
	struct led_base base;
	uint8_t         pin;
	bool            on_level;
};

struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};

struct led_i2c {
	struct led_base base;
	uint8_t         bus;
	uint8_t         addr;
};
```

每个子类的实现函数第一行都是 container_of：

```c
static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	return 0;
}
```

注意 `platform_gpio_write`，子类不直接碰寄存器，调 platform 层的封装函数。封装函数内部走的是 ops 分发（下一节展开），但驱动层这里只见普通函数。

GPIO 子类只填 `on / off`，`set_brightness` 留空，GPIO 灯没有调光能力。PWM 子类三件套全填。I2C 子类填 `on / off`。

```c
static const struct led_ops gpio_ops = { .on = gpio_on, .off = gpio_off };

static const struct led_ops pwm_ops = {
	.on             = pwm_on,
	.off            = pwm_off,
	.set_brightness = pwm_set_brightness,
};

static const struct led_ops i2c_ops = { .on = i2c_on, .off = i2c_off };
```

![子类代码](../assets/ch15/slide3_子类代码.png)

## 15.4 板级层：唯一认识硬件的文件

`board_init.c`，整个工程里唯一认识硬件的文件：

```c
static struct led_gpio s_led_err;       /* 文件作用域，外部不可见 */
static struct led_pwm  s_led_status;
static struct led_i2c  s_led_net;

struct led_base *g_led_error;
struct led_base *g_led_status;
struct led_base *g_led_network;

void board_init(void)
{
	led_gpio_init(&s_led_err,    "ERR",  10, true);
	led_pwm_init (&s_led_status, "STAT",  1, 50);
	led_i2c_init (&s_led_net,    "NET",   0, 0x20);

	g_led_error   = &s_led_err.base;
	g_led_status  = &s_led_status.base;
	g_led_network = &s_led_net.base;
}
```

报警灯用 GPIO，状态灯用 PWM，网络灯挂在 I2C 扩展芯片上。三种不同硬件，通过同一个 `struct led_base *` 对外暴露。

![板级代码](../assets/ch15/slide4_板级代码.png)

## 15.5 应用层：grep 零硬件字样

`app.c`：

```c
#include "leds.h"

void alarm_blink(void)
{
	led_on(g_led_error);
	led_off(g_led_error);
}

void status_breathe(void)
{
	led_set_brightness(g_led_status, 30);
	led_set_brightness(g_led_status, 80);
	led_set_brightness(g_led_status,  0);
}

void power_on_test(void)
{
	led_on(g_led_error);    led_off(g_led_error);
	led_on(g_led_status);   led_off(g_led_status);
	led_on(g_led_network);  led_off(g_led_network);
}
```

打开终端：

```
grep -n "led_gpio\|led_pwm\|led_i2c"  app.c    # 0
grep -n "gpio_write\|HAL_GPIO\|sysfs" app.c    # 0
grep -n "BSRR\|0x[0-9A-F]"            app.c    # 0
```

应用层一个硬件字样都没有。

![应用层 + grep](../assets/ch15/slide5_应用层grep.png)

## 15.6 platform 层的内部演化：从直接实现到 ops 分发

到这里应用层、led 驱动层、板级层全部 ops 化了。还差一层：platform 层。

ch01 到 ch10 platform 层是直接实现，没有 ops 表：

```c
/* common/platform.h - 对外接口 */
void platform_gpio_init(uint8_t pin, uint8_t mode);
void platform_gpio_write(uint8_t pin, bool value);
bool platform_gpio_read(uint8_t pin);

/* common/platform_pc.c - 直接实现 */
void platform_gpio_write(uint8_t pin, bool value)
{
	printf("[GPIO] Pin%u -> %s\n", pin, value ? "HIGH" : "LOW");
}
```

这一版有一个硬限制：链 `platform_pc.c` 这份目标文件就跑 PC 实现，链 `platform_stm32.c` 就跑 STM32 实现。运行时切换不了。

但工业项目里有一个高频需求：**同一个二进制文件，启动期或测试期决定用哪一套 GPIO 实现**。比如 unit test 要在 CI 流水线里跑，模拟环境用 PC 实现，回到真机用 STM32 实现，源码 0 改、binary 0 改、只换一个全局指针。

这一节就把 platform 层重新组织成 ops 分发版。

### 15.6.1 关键不变量：对外接口一字不改

```c
/* common/platform.h - 这一份从 ch01 到 ch20 一字不改 */
void platform_gpio_init(uint8_t pin, uint8_t mode);
void platform_gpio_write(uint8_t pin, bool value);
bool platform_gpio_read(uint8_t pin);
```

驱动层、应用层调的就是这几个普通 C 函数。从 ch01 起到 ch15、到附录 B 的真实 STM32 工程，签名 0 变化。

这是 SOLID 接口隔离原则在 platform 层的落地：上层不知道也不该知道下层是直接实现还是 ops 分发。

#### pin 编码：把 port 藏进单参数

`platform_gpio_write(uint8_t pin, bool value)` 这个签名只有一个 `pin` 参数。但真实 MCU 上 GPIO 是 port (A/B/C/D...) + 引脚号 (0-15) 两层结构。怎么用一个参数表达？

工业代码里 `pin` 是个编码值。最简单的编码：

```c
/* 高 4 位放 port 索引（A=0, B=1, C=2, D=3, ...），低 4 位放引脚号 */
#define PIN(port, num)   (((port) << 4) | ((num) & 0xF))
#define PIN_PORT(pin)    (((pin) >> 4) & 0xF)
#define PIN_NUM(pin)     ((pin) & 0xF)

#define PORT_A  0
#define PORT_B  1
#define PORT_C  2
#define PORT_D  3
```

调用方写：

```c
platform_gpio_write(PIN(PORT_D, 12), true);    /* PD12 高 */
platform_gpio_write(PIN(PORT_A, 5),  false);   /* PA5 低 */
```

platform 层内部解码：

```c
static void stm32_gpio_write(uint8_t pin, bool value)
{
	GPIO_TypeDef *gpio = port_to_gpio(PIN_PORT(pin));   /* 0 -> GPIOA */
	uint16_t mask = (1u << PIN_NUM(pin));
	HAL_GPIO_WritePin(gpio, mask,
			  value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
```

这一招让接口**永远只有一个 `pin` 参数**，无论底下是单 port 的小芯片还是 12 个 port 的大芯片。port 信息只在 platform 层内部解码，上层完全感知不到。

工业级落地：

| 项目 | 接口签名 | port 处理 |
|---|---|---|
| 本书 / 附录 B | `platform_gpio_write(uint8_t pin, bool)` | `PIN(port, num)` 编码 |
| Linux 内核 | `gpio_set_value(unsigned int gpio, int)` | gpio 是全局编号，gpiochip 注册时 base + offset |
| 工业控制板 | `platform_pin_write(int32_t pin, int32_t)` | `GET_PIN(PORTx, PIN)` 编码（位移 + 端口宏） |
| Zephyr | `gpio_pin_set_dt(spec, value)` | spec 是设备树编译出的句柄，包含 port + pin |
| STM32 HAL（反例） | `HAL_GPIO_WritePin(GPIO_TypeDef*, uint16_t, ...)` | 暴露 GPIO_TypeDef 指针给驱动层 |

前四个都把 port 藏起来了，HAL 是反例。这就是为什么直接基于 HAL 写驱动很难做跨芯片移植。这本书 + Linux + 工业控制板都站在"封装 port"这一边。

### 15.6.2 platform 层内部三个文件

ch15 把 platform 层拆成三块：

```
platform_ops.h            内部 ops 表 + 三个 const 实例的声明
platform_dispatch.c       封装函数实现（platform_gpio_xxx）+ 切换 API
platform_ops_pc.c         PC 实例
platform_ops_stm32_mock.c STM32 实例
platform_ops_linux_mock.c Linux 实例
```

`platform_ops.h` 是 platform 层**内部**头文件，只有 platform 层自己 include。驱动层永远 include `platform.h`（封装函数声明），永远不 include `platform_ops.h`。

```c
/* platform_ops.h - 只给 platform 层自己用 */
struct platform_ops {
	const char *name;
	void (*gpio_init)(uint8_t pin, uint8_t mode);
	void (*gpio_write)(uint8_t pin, bool value);
	bool (*gpio_read)(uint8_t pin);
	void (*gpio_deinit)(uint8_t pin);
};

extern const struct platform_ops platform_pc;
extern const struct platform_ops platform_stm32_mock;
extern const struct platform_ops platform_linux_mock;

void platform_select(const struct platform_ops *p);
const char *platform_name(void);
```

### 15.6.3 platform_dispatch.c：封装函数走 ops 分发

`platform_dispatch.c` 维护 platform 层内部的 ops 指针，并实现对外的封装函数：

```c
/* platform_dispatch.c */
#include "platform.h"
#include "platform_ops.h"

static const struct platform_ops *g_platform_ops = &platform_pc;

void platform_select(const struct platform_ops *p)
{
	if (p) {
		g_platform_ops = p;
		printf(">>> platform switched to: %s\n", p->name);
	}
}

const char *platform_name(void)
{
	return g_platform_ops ? g_platform_ops->name : "(unset)";
}

void platform_gpio_write(uint8_t pin, bool value)
{
	if (g_platform_ops && g_platform_ops->gpio_write)
		g_platform_ops->gpio_write(pin, value);
}

/* platform_gpio_init / read / deinit 同套路 */
```

`g_platform_ops` 是 platform 层的私有状态，文件作用域，外部链接不到。换平台只改这一个指针。

### 15.6.4 三个 ops 实例

ch15 同时编译进三个 `platform_ops` 实例。三份骨架完全一致，只是函数体不同。

PC 版（`platform_ops_pc.c`）：

```c
static void pc_gpio_write(uint8_t pin, bool value)
{
	printf("    [PC]    Pin%u <- %s\n", (unsigned)pin,
	       value ? "HIGH" : "LOW");
}

const struct platform_ops platform_pc = {
	.name       = "PC",
	.gpio_init  = pc_gpio_init,
	.gpio_write = pc_gpio_write,
	.gpio_read  = pc_gpio_read,
};
```

PC 实例文件里只定义这一个 const 实例，不维护任何全局状态。"默认指向 PC"这件事在 `platform_dispatch.c` 里完成（`g_platform_ops = &platform_pc`）。

STM32 mock 版（`platform_ops_stm32_mock.c`）打印 BSRR 寄存器写入动作：

```c
static void stm32_gpio_write(uint8_t pin, bool value)
{
	/* 真机上这里就是：
	 *   GPIOA->BSRR = value ? (1u << pin) : (1u << (pin + 16));
	 * BSRR 一次 32 位 store，原子。 */
	printf("    [STM32] BSRR <- 0x%08X (Pin%u %s)\n",
	       value ? (1u << pin) : (1u << (pin + 16)),
	       (unsigned)pin, value ? "HIGH" : "LOW");
}

const struct platform_ops platform_stm32_mock = {
	.name       = "STM32",
	.gpio_init  = stm32_gpio_init,
	.gpio_write = stm32_gpio_write,
	.gpio_read  = stm32_gpio_read,
};
```

Linux mock 版（`platform_ops_linux_mock.c`）打印 sysfs 文件写入动作：

```c
static void linux_gpio_write(uint8_t pin, bool value)
{
	printf("    [LINUX] echo %d > /sys/class/gpio/gpio%u/value\n",
	       value ? 1 : 0, (unsigned)pin);
}

const struct platform_ops platform_linux_mock = {
	.name       = "LINUX",
	.gpio_init  = linux_gpio_init,
	.gpio_write = linux_gpio_write,
	.gpio_read  = linux_gpio_read,
};
```

三个实例都是 `const`，存在 .rodata 段。运行时切换只是改 platform 层内部的 `g_platform_ops` 指针指向（`platform_select` 实现见 15.6.3）。

### 15.6.5 工业纪律：上层永远调封装函数

回头看 led 子类的实现：

```c
static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, self->on_level);    /* 普通 C 函数调用 */
	me->is_on = true;
	return 0;
}
```

驱动层调的是 `platform_gpio_write(pin, value)`，不是 `g_platform_ops->gpio_write(pin, value)`。封装函数内部走 ops 分发是 platform 层的实现细节，对驱动层不可见。

这条纪律和 ch01 1.10 节 `led_on(green_led)` 同源：**应用层永远只见普通函数，看不到 ops 表**。两者放一起就是这本书的核心工业纪律：

| 层 | 调用形态 | 内部 |
|---|---|---|
| 应用层 | `led_on(handle)` | 父类封装函数走 led ops 分发到子类 |
| 驱动层 | `platform_gpio_write(pin, val)` | platform 封装函数走 platform ops 分发到具体平台 |
| platform 层 / led 父类 | 内部维护 ops 指针 + dispatch | 私有状态，外部链接不到 |
| ops 实例 | `const struct xxx_ops` 在 .rodata | 直接实现，不再有第二层间接 |

每一层只见下一层暴露的封装函数。ops 表只在自己这一层内部用，永远不跨层暴露。

这是 SOLID 接口隔离（ISP）+ 依赖倒置（DIP）在 C 里的工程化表达。Linux 内核的 `gpio_request / gpio_set_value` / Linux drivers 的 `i2c_transfer` / Zephyr 的 `gpio_pin_set_dt` 全部是这个形态：对外是普通 C 函数，内部是各家芯片厂的 ops 实例。

## 15.7 main.c：同一份业务跑三遍

```c
int main(void)
{
	board_init();    /* 一次构造，三种 platform 通用 */

	printf("\n========== run on PC ==========\n");
	platform_select(&platform_pc);
	power_on_test();
	alarm_blink();
	status_breathe();

	printf("\n========== run on STM32 ==========\n");
	platform_select(&platform_stm32_mock);
	power_on_test();
	alarm_blink();
	status_breathe();

	printf("\n========== run on LINUX ==========\n");
	platform_select(&platform_linux_mock);
	power_on_test();
	alarm_blink();
	status_breathe();
	return 0;
}
```

输出节选：

```
========== run on PC ==========
>>> platform switched to: PC

--- alarm_blink ---
    [PC]    Pin10 <- HIGH
  [ERR] led_on -> platform=PC
    [PC]    Pin10 <- LOW

========== run on STM32 ==========
>>> platform switched to: STM32

--- alarm_blink ---
    [STM32] BSRR <- 0x00000400 (Pin10 HIGH)
  [ERR] led_on -> platform=STM32
    [STM32] BSRR <- 0x04000000 (Pin10 LOW)

========== run on LINUX ==========
>>> platform switched to: LINUX

--- alarm_blink ---
    [LINUX] echo 1 > /sys/class/gpio/gpio10/value
  [ERR] led_on -> platform=LINUX
    [LINUX] echo 0 > /sys/class/gpio/gpio10/value
```

应用层代码、led 驱动层代码、board 层代码，全部 0 修改。换的只有 platform 全局指针指向的那个 ops 实例。

## 15.8 换硬件 diff

真实场景：周五下午老板进来，客户改要求，报警灯要能调光，从 GPIO 换成 PWM。

打开 `board_init.c`，改 3 行：

```c
/* 改前 */
static struct led_gpio s_led_err;
led_gpio_init(&s_led_err, "ERR", 10, true);
g_led_error = &s_led_err.base;

/* 改后 */
static struct led_pwm s_led_err;
led_pwm_init(&s_led_err, "ERR", 2, 80);
g_led_error = &s_led_err.base;
```

`app.c` 0 改动。`led.c` 0 改动。`platform_*.c` 0 改动。

如果是换底层芯片（比如从 STM32 换到瑞萨 RA），`platform_ops_stm32.c` 替换成 `platform_ops_ra.c`，上面三层全部 0 改动。

![换硬件 diff](../assets/ch15/slide6_换硬件diff.png)

## 15.9 Before / After

最初你的代码是这样：

```c
void red_led_on(void)  { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_SET); }
void red_led_off(void) { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_RESET); }
void green_led_on(void)  { ... }
void green_led_off(void) { ... }
/* 8 个 LED 乘 4 个函数 = 32 个几乎一模一样的函数 */
```

应用层和 HAL 库直接耦合。换硬件？认真考虑辞职。

现在你的代码：

```c
/* app.c */
led_on(g_led_error);
led_set_brightness(g_led_status, 80);
```

四层架构。每一层只关心自己。换硬件改 board_init 三行，应用 0 修改。

代码行数从 300 多行（重复的 HAL 调用）压到应用层 60 行。

但**真正重要的不是行数少，是改一处全生效**。

![Before / After](../assets/ch15/slide7_BeforeAfter.png)

## 15.10 视频里没讲透的几个细节

### 15.10.1 ops 分发版的间接代价

封装函数 `platform_gpio_write(pin, value)` 在 ch15 这一版的实现是：

```c
void platform_gpio_write(uint8_t pin, bool value)
{
	if (g_platform_ops && g_platform_ops->gpio_write)
		g_platform_ops->gpio_write(pin, value);
}
```

调用方调一次普通函数，函数体内多两次 load + 一次间接跳：

```
1. 取 g_platform_ops 全局变量的当前值（一次 load）
2. 取 g_platform_ops->gpio_write 字段的值（一次 load）
3. 间接跳到那个函数地址
```

ARM Cortex-M 上多两条指令，约 2 个周期。和 ch01 到 ch10 的"直接实现版"相比，单次调用慢 2 个周期。换到的是运行时切换平台 + 同一份 binary 跑测试 platform + 真机 platform 的能力。

如果你的项目永远不切换平台，直接实现版也够用（ch01 到 ch10 一直在用）。但读到这一章的工程师多半不是教学项目，能在 CI 流水线里跑 unit test 会让你受益匪浅。

### 15.10.2 platform_ops 一定要 const + .rodata

```c
const struct platform_ops platform_pc = {
	.gpio_init  = pc_gpio_init,
	/* ... */
};
```

`const` 让这个实例进 .rodata 段（只读）。两个理由：

1. **崩溃定位**：如果有错代码踩到这个实例（比如把 `gpio_write` 改成野指针），立刻段错误，比静默崩好。
2. **平台并存**：三个实例都在 .rodata 里，互不干扰。`platform_select` 只改 platform 层内部 `g_platform_ops` 指针的指向。

### 15.10.3 子类层不知道 platform 是谁

注意 `gpio_on` 里这一行：

```c
platform_gpio_write(self->pin, self->on_level);
```

子类层调的是封装函数 `platform_gpio_write`，从来不引用 `platform_pc` / `platform_stm32` 等具体实例，也不知道封装函数内部走的是 ops 分发还是直接实现。子类只知道有一个名叫 `platform_gpio_write` 的普通 C 函数在 platform.h 里声明着，至于背后是谁、怎么实现，不关心。

这一招不仅让 platform 可换，还让子类层自动跨平台。led_gpio.c 在 PC、STM32、Linux 上都能编译运行，源码 0 修改。

### 15.10.4 板级初始化 vs platform 选择的边界

`board_init.c` 决定 LED 接什么硬件（GPIO / PWM / I2C）。`platform_select` 决定 GPIO 物理操作落在哪里（PC printf / STM32 BSRR / Linux sysfs）。

两个职责不重合：

- 同一块板子（同一份 board_init），可以在 PC 跑测试、在 STM32 跑硬件版，只换 platform。
- 同一个 platform（比如 STM32），可以服务多种板子（不同 board_init），多产品线共享一份 platform 层。

这是 1×N 变 N+M 的关键。

### 15.10.5 platform 切换的线程安全

如果项目里有多个线程同时调 `platform_gpio_write`，主线程突然 `platform_select(other)`，封装函数体里那一行 `g_platform_ops->gpio_write(...)` 会不会和别的线程的 `gpio_write` 调用打架？

C 标准里指针赋值不保证原子（虽然在主流 32 位 / 64 位平台上对齐的指针读写都是原子的）。生产代码里要么：

1. 启动期一次性选定 platform，运行时不再换。
2. 加锁或用 `_Atomic`（C11）保护 `g_platform_ops` 指针。

多数 RTOS 项目走方案 1：开机 platform 选定，之后整个项目不再切换。"运行时切换"主要是开发期 unit test 用（同一份代码在 PC 模拟环境和真机间切换）。

### 15.10.6 1×N vs N×N 的算账

第 16 章会展开这个图，先放结论：

- **没有 platform 层**：3 种设备驱动 × 5 家芯片 = 15 份代码。加一家芯片再写 3 份。
- **有 platform 层**：3 种设备驱动 + 5 家 platform 适配 = 8 份代码。加一家芯片只写 1 份 platform 适配，驱动 0 改。

乘法变加法。Linux 内核几万个驱动能在不同芯片上跑，就是这一招的工业级展开。

### 15.10.7 板级配置文件：设备树的雏形

`board_init.c` 这个文件回答的问题是："这块板子上有几颗 LED？分别什么类型？接在哪个引脚？"

把它的"硬件描述"部分单拎出来看：

```c
/* board_init.c 节选，硬件描述部分 */
static struct led_gpio s_led_err;     /* 报警灯：GPIO 类型，pin 10 */
static struct led_pwm  s_led_status;  /* 状态灯：PWM 类型，channel 1 */
static struct led_i2c  s_led_net;     /* 网络灯：I2C，bus 0 addr 0x20 */
```

这些信息（"哪个角色挂哪个硬件"）有一个共性：和应用层无关，和 platform 层无关，只描述这块板子上插了什么。

工业项目上做大了之后，这些信息会被抽到一个独立的"板级配置目录"：

```
component_cfg/
├── led_cfg.h         /* LED 角色 → 硬件参数映射 */
├── motor_cfg.h       /* Motor 角色 → 硬件参数映射 */
├── sensor_cfg.h      /* 传感器角色 → 硬件参数映射 */
└── board_init.c      /* 引用上面的 cfg 实例化 */
```

每块板子一份 `component_cfg/`。换板子（同 SoC、不同硬件方案），改 `component_cfg/` 这一个目录，其它代码 0 改。

这就是 Linux / Zephyr 用的**设备树（device tree）的雏形**。设备树进一步把这些 cfg 信息从 C 代码挪到一个文本文件（`.dts`），编译期编进 ROM，启动期内核解析这棵树，逐节点找到驱动并挂上 ops。换硬件不改 C 代码、只改 .dts。这本书 ch16 会再展开一次。

裸机或 RTOS 项目还没必要做到 dts 那么彻底，但"硬件描述独立成一个目录"这一招值得现在就开始。`component_cfg/` 是工程师能写出来的、最早一版"设备树"。

### 15.10.8 设备的注册与查找：platform_device_register / find

ch12 / ch15 你写的 `g_led_error`、`g_led_status` 这种全局句柄，到了项目规模够大之后会变得难管：

- 项目里有 50 个全局句柄，每个都得在 `leds.h` 里 `extern` 一份。
- 加一个新设备就改 `leds.h`，`leds.h` 越来越胖。
- 主从板项目里需要"运行时动态加挂设备"，全局句柄不够用。

工业项目和 Linux 内核走的是另一条路：**设备注册到一张表里，调用方按名字查找**。

```c
/* drivers/led/led_register.h（伪代码风格） */
int  led_register(const char *name, struct led_base *handle);
struct led_base *led_find(const char *name);
```

`board_init.c` 改成：

```c
void board_init(void)
{
	led_gpio_init(&s_led_err,    "ERR",  10, true);
	led_pwm_init (&s_led_status, "STAT",  1, 50);
	led_i2c_init (&s_led_net,    "NET",   0, 0x20);

	led_register("error",   &s_led_err.base);
	led_register("status",  &s_led_status.base);
	led_register("network", &s_led_net.base);
}
```

应用层不再需要 extern 任何句柄：

```c
/* app.c */
struct led_base *led = led_find("error");
if (led) led_on(led);
```

按名字而非全局变量取设备，这是 Linux 内核 `platform_device_register` / `device_find_child_by_name` / `bus_find_device_by_name` 这一族 API 的精神。一张设备注册表，按字符串 key 查询。

代价是多了一层间接（每次调用前先 find 一次），多数项目把 find 结果缓存在静态变量里抹平这个开销。

何时升级到这一招？项目里全局句柄超过 20 个、或者设备需要运行时动态加挂（USB 设备热插拔、I2C 探测）的时候。本书早期章节用全局句柄，是教学简化。真实工业代码里 `led_register / led_find` 风格更常见。

### 15.10.9 设备驱动 + 中间件：四层之间还有一层

到目前为止你看到的四层是"应用 / LED 驱动 / 板级 / Platform"。但工业项目里"驱动"和"应用"之间通常还有一层：**中间件**（middleware）。

例子：

- **状态指示器中间件**：报警灯、状态灯、网络灯三盏 LED 的高级行为（呼吸、闪烁、组合编码），不是直接在 app.c 里写 `led_on(g_led_error); sleep(100); led_off(g_led_error);`，而是放进 `indicator/`：

```c
/* middleware/indicator/indicator.h */
void indicator_set_alarm(enum alarm_state state);   /* OK / WARN / ERROR / FAULT */
void indicator_set_network(bool connected);
void indicator_run_self_test(void);
```

`indicator.c` 内部知道要操作哪些 LED、什么节奏、用 PWM 还是 GPIO。app.c 调用的是"业务语义"（"报警"、"网络断连"），不是"硬件操作"（"点亮 / 熄灭"）。

- **存储中间件**：`storage_write(key, value)` 内部决定写到 EEPROM 还是 SPI Flash 还是文件系统。
- **通讯中间件**：`comm_send(msg)` 内部决定走 UART 还是 CAN 还是 Ethernet。

中间件层的存在让分层完整：

```
应用层      app.c             业务流程，决策"什么时候报警"
中间件层    indicator.c       业务到硬件的桥梁，决策"报警 = 红灯 PWM 50% 1Hz 闪烁"
LED 驱动层  led.c             多态 dispatch，操作单颗 LED
板级层      board_init.c      实例化、注册
Platform 层 platform_*.c      物理操作
```

5 层。每一层只调下一层。这就是工业代码里"业务和硬件解耦得彻底"的做法。

中间件不是凭空多出来的"过度设计"。它的判据是：**当业务语义和硬件操作不是一对一映射的时候**，中间件就值得引入。"报警 → 三盏灯协同（红灯主报、绿灯熄灭、蜂鸣器响 1 秒）"是一对多映射，写在 app 里 app 就有了硬件知识，写在 led.c 里 led.c 就有了业务知识，都不对。中间件吸收这个错配。

ch15 的配套代码没有中间件层（教学简化），ch19 的工业实战章会展开一个完整的 indicator 中间件案例。

### 15.10.10 platform_ops 的字段规模

本章 platform_ops 只有 4 个字段（init / write / read / name）。真实工业项目里规模大很多：

```c
struct platform_ops {
	const char *name;

	/* GPIO */
	void (*gpio_init)(uint8_t pin, uint8_t mode);
	void (*gpio_write)(uint8_t pin, bool value);
	bool (*gpio_read)(uint8_t pin);

	/* I2C */
	int (*i2c_transfer)(uint8_t bus, uint8_t addr,
			    const uint8_t *tx, size_t tx_len,
			    uint8_t *rx, size_t rx_len);

	/* SPI */
	int (*spi_transfer)(uint8_t bus, const uint8_t *tx,
			    uint8_t *rx, size_t len);

	/* UART */
	int (*uart_write)(uint8_t port, const uint8_t *buf, size_t len);
	int (*uart_read)(uint8_t port, uint8_t *buf, size_t max);

	/* PWM */
	void (*pwm_set_duty)(uint8_t channel, uint8_t duty);

	/* ADC */
	int (*adc_read)(uint8_t channel, uint16_t *value);

	/* timing */
	void (*delay_us)(uint32_t us);
	uint32_t (*get_tick_ms)(void);
};
```

20-30 个字段。每家芯片提供一份完整实现。

字段太多了拆不拆？看你项目。SOLID 接口隔离原则会建议拆成 `gpio_ops / i2c_ops / spi_ops / ...` 多个小表。Linux 内核走的就是这条路（没有一个大 platform_ops，而是一堆 subsystem 各自的 ops 表）。本书 ch16 会展开 Linux 的多个 ops 表。

## 15.11 你现在的代码在 STM32 上长什么样

`stm32-snippet/platform_ops_stm32.c` 是真实硬件版的 `platform_ops` 实例：

```c
static void stm32_gpio_write(uint8_t pin, bool value)
{
	HAL_GPIO_WritePin(GPIOA, (uint16_t)(1U << pin),
			  value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

const struct platform_ops platform_stm32 = {
	.name       = "STM32-real",
	.gpio_init  = stm32_gpio_init,
	.gpio_write = stm32_gpio_write,
	.gpio_read  = stm32_gpio_read,
};
```

启动期 `main` 调一次 `platform_select(&platform_stm32)`，platform 层内部 `g_platform_ops` 指向这个实例，之后所有 `platform_gpio_write(...)` 调用都落到 `stm32_gpio_write`。

`HAL_GPIO_WritePin` 调到底就是写 BSRR 寄存器：

```c
GPIOx->BSRR = (uint32_t)GPIO_Pin;          /* SET */
GPIOx->BSRR = (uint32_t)GPIO_Pin << 16;    /* RESET */
```

应用层、led 驱动层、board 层一字不改。封装函数 `platform_gpio_write` 的签名一字不改。

## 15.12 你现在的代码在 Linux 用户态长什么样

`linux-snippet/platform_ops_linux.c` 是真实 Linux 用户态版本，sysfs 写文件：

```c
static void linux_gpio_write(uint8_t pin, bool value)
{
	char path[64];
	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%u/value", (unsigned)pin);
	int fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, value ? "1" : "0", 1);
		close(fd);
	}
}

const struct platform_ops platform_linux = {
	.name       = "LINUX-real",
	/* ... */
};
```

启动期 `main` 调 `platform_select(&platform_linux)` 选定即可。更现代的方案是 libgpiod，附录 C 会展开。

## 15.13 工业代码里的 platform 层长什么样

工业控制板项目里 platform 层就是 ops 分发，跨 5 套产品共用一份抽象。对外暴露的是普通封装函数：

```c
/* drivers/platform/platform.h - 上层只 include 这一份 */
void platform_gpio_init(uint8_t pin, const struct gpio_cfg *cfg);
void platform_gpio_write(uint8_t pin, bool value);
bool platform_gpio_read(uint8_t pin);
int  platform_i2c_transfer(uint8_t bus, uint8_t addr,
			   const uint8_t *tx, size_t tx_len,
			   uint8_t *rx, size_t rx_len);
int  platform_adc_read(uint8_t channel, uint16_t *value);
uint32_t platform_get_tick_ms(void);
```

ops 表是 platform 层内部细节，放在 platform 自己的 internal 头里：

```c
/* drivers/platform/platform_ops.h - 只给 platform 层自己用 */
struct platform_ops {
	void (*gpio_init)(uint8_t pin, const struct gpio_cfg *cfg);
	void (*gpio_write)(uint8_t pin, bool value);
	bool (*gpio_read)(uint8_t pin);
	int  (*i2c_transfer)(uint8_t bus, uint8_t addr,
			     const uint8_t *tx, size_t tx_len,
			     uint8_t *rx, size_t rx_len);
	int  (*adc_read)(uint8_t channel, uint16_t *value);
	uint32_t (*get_tick_ms)(void);
	/* ... */
};
```

每一款主控芯片提供一份 platform 实现：

```
drivers/platform/
├── platform.h               /* 对外封装函数声明 */
├── platform_ops.h           /* 内部 ops 结构 + 实例 extern */
├── platform_dispatch.c      /* 封装函数实现，内部走 ops 分发 */
├── stm32f4/platform_stm32f4.c       /* 一款主控的 ops 实例 */
├── stm32h7/platform_stm32h7.c       /* 另一款主控的 ops 实例 */
└── pc_sim/platform_pc.c             /* PC 单元测试用 */
```

5 套产品（不同板型）、3 款主控芯片，driver 模块（led / motor / encoder / sensor / eeprom）写一遍跨产品共享。换板子换芯片只改 platform 这一层的 ops 实例。

这是 platform 抽象在工业项目里的最终形态：**对外是普通 C 函数，对内是 ops 分发，平台 + 业务双向解耦**。

## 15.14 完整源码清单

把下面的代码块分别保存到对应的文件，目录结构和 [`oop-in-c/code/15-platform/pc/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/oop-in-c/code/15-platform/pc/) 一致。`make && ./demo` 即可跑通。

本章引入了三个 `platform_ops` 实例 + `platform_dispatch.c` 封装函数 + `platform_ops.h` 内部头文件，全部 11 个文件贴齐。

### 文件 1：`main.c`（49 行）

应用层入口。`board_init` 调一次，三种 platform（PC / STM32 mock / Linux mock）轮流跑同一份业务流。

```c
/* SPDX-License-Identifier: MIT */
/*
 * main.c - 运行时切换 platform 演示
 *
 * 同一份业务代码，跑三次，分别在 PC / STM32 / Linux 三种 platform 下执行。
 * 应用层、led 驱动层、board 层一字不改，只换 platform 全局指针。
 *
 * 这是 ch15 的核心：分层抽象的高潮。
 */

#include "app.h"
#include "leds.h"
#include "platform_ops.h"
#include <stdio.h>

int main(void)
{
	printf("=========================================\n");
	printf("  ch15 - platform abstraction climax\n");
	printf("=========================================\n");

	board_init();    /* 一次构造，三种 platform 通用 */

	printf("\n========== run on PC ==========\n");
	platform_select(&platform_pc);
	power_on_test();
	alarm_blink();
	status_breathe();

	printf("\n========== run on STM32 ==========\n");
	platform_select(&platform_stm32_mock);
	power_on_test();
	alarm_blink();
	status_breathe();

	printf("\n========== run on LINUX ==========\n");
	platform_select(&platform_linux_mock);
	power_on_test();
	alarm_blink();
	status_breathe();

	printf("\n=========================================\n");
	printf("  same app/led/board, 3 platforms\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
```

### 文件 2：`app.h`（9 行）

应用层三个业务函数声明。

```c
/* SPDX-License-Identifier: MIT */
#ifndef APP_H
#define APP_H

void alarm_blink(void);
void status_breathe(void);
void power_on_test(void);

#endif
```

### 文件 3：`app.c`（34 行）

三个业务函数实现。grep 这一份文件 gpio_write / pwm_ / i2c_ / HAL_ / sysfs，结果都是 0。

```c
/* SPDX-License-Identifier: MIT */
/*
 * app.c - 应用层
 *
 * 三个业务函数。整个文件里 grep gpio_write / pwm_ / i2c_ / HAL_ / sysfs：
 * 全部 0。应用层不认识硬件，硬件是谁它都不问。
 */

#include "leds.h"
#include <stdio.h>

void alarm_blink(void)
{
	printf("\n--- alarm_blink ---\n");
	led_on(g_led_error);
	led_off(g_led_error);
}

void status_breathe(void)
{
	printf("\n--- status_breathe ---\n");
	led_set_brightness(g_led_status, 30);
	led_set_brightness(g_led_status, 80);
	led_set_brightness(g_led_status,  0);
}

void power_on_test(void)
{
	printf("\n--- power_on_test ---\n");
	led_on(g_led_error);    led_off(g_led_error);
	led_on(g_led_status);   led_off(g_led_status);
	led_on(g_led_network);  led_off(g_led_network);
}
```

### 文件 4：`leds.h`（13 行）

板级对外暴露的全局 LED 句柄声明。

```c
/* SPDX-License-Identifier: MIT */
#ifndef LEDS_H
#define LEDS_H

#include "led.h"

extern struct led_base *g_led_error;
extern struct led_base *g_led_status;
extern struct led_base *g_led_network;

void board_init(void);

#endif
```

### 文件 5：`board_init.c`（28 行）

整个工程里唯一认识硬件的文件。三种 LED 子类对象在文件作用域定义，`board_init` 跑各自构造函数 + 把 `&xxx.base` 赋给全局父类句柄。

```c
/* SPDX-License-Identifier: MIT */
/*
 * board_init.c - 板级硬件配置
 *
 * 唯一认识硬件的文件。三种子类混搭。
 */

#include "leds.h"

static struct led_gpio s_led_err;
static struct led_pwm  s_led_status;
static struct led_i2c  s_led_net;

struct led_base *g_led_error;
struct led_base *g_led_status;
struct led_base *g_led_network;

void board_init(void)
{
	led_gpio_init(&s_led_err,    "ERR",  10, true);
	led_pwm_init (&s_led_status, "STAT",  1, 50);
	led_i2c_init (&s_led_net,    "NET",   0, 0x20);

	g_led_error   = &s_led_err.base;
	g_led_status  = &s_led_status.base;
	g_led_network = &s_led_net.base;
}
```

### 文件 6：`led.h`（62 行）

LED 父类 + 三个子类的声明。这一版字段定型，整本书 ch12+ 都用这一版。

```c
/* SPDX-License-Identifier: MIT */
/*
 * led.h - LED 驱动层（ch15 完整版）
 *
 * 应用层只用 led_base 句柄。led_gpio / led_pwm / led_i2c 是子类。
 * 子类实现里调 platform_gpio_xxx 封装函数，从来不直接碰寄存器。
 *
 * 这一层只关心"GPIO 能写吗、I2C 能传吗"，硬件细节交给 platform 层。
 */

#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdbool.h>

struct led_base;

struct led_ops {
	int (*on)(struct led_base *me);
	int (*off)(struct led_base *me);
	int (*set_brightness)(struct led_base *me, uint8_t brightness);
};

struct led_base {
	const struct led_ops *ops;
	const char           *name;
	bool                  is_on;
};

int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_set_brightness(struct led_base *me, uint8_t brightness);

/* GPIO 子类 */
struct led_gpio {
	struct led_base base;
	uint8_t         pin;
	bool            on_level;
};
void led_gpio_init(struct led_gpio *me, const char *name,
		   uint8_t pin, bool on_level);

/* PWM 子类 */
struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};
void led_pwm_init(struct led_pwm *me, const char *name,
		  uint8_t channel, uint8_t duty);

/* I2C 子类 */
struct led_i2c {
	struct led_base base;
	uint8_t         bus;
	uint8_t         addr;
};
void led_i2c_init(struct led_i2c *me, const char *name,
		  uint8_t bus, uint8_t addr);

#endif /* LED_H */
```

### 文件 7：`led.c`（172 行）

LED 驱动层实现。父类必填 + 选填 + 三个子类。子类只调 `platform_gpio_xxx` 等封装函数，从来不引用 `g_platform_ops` 内部指针。日志里 `platform_name()` 让你看到"同一行业务代码、三种 platform 出三种格式"。

```c
/* SPDX-License-Identifier: MIT */
/*
 * led.c - LED 驱动层实现
 *
 * 注意：这一层永远只调 platform 层的封装函数（platform_gpio_write /
 * platform_gpio_init 等）。从来不直接碰 platform_ops 字段。
 *
 * 切换平台的能力由 platform 层内部完成（platform_dispatch.c 的
 * g_platform_ops 指针），驱动层一字不知。这一层在 PC、STM32、Linux
 * 上都能编译运行，源码 0 修改。
 *
 * 这是工业代码里"对外稳定、对内可换"的纪律：上层永远调封装函数，
 * 内部分发是 platform 层的实现细节。
 */

#include "led.h"
#include "container_of.h"
#include "platform.h"
#include "platform_ops.h"
#include <assert.h>
#include <stdio.h>

/* ============== 父类统一接口（必填 + 选填） ============== */

int led_on(struct led_base *me)
{
	if (!me)
		return -1;
	/* on 是必填：子类必须实现。调试构建里 assert 抓到忘填的子类
	 * 立刻 abort 给行号；Release 构建定义 NDEBUG 后 assert 整行消失，
	 * 零运行时开销。 */
	assert(me->ops && me->ops->on &&
	       "led_on: subclass must implement on()");
	return me->ops->on(me);
}

int led_off(struct led_base *me)
{
	if (!me)
		return -1;
	assert(me->ops && me->ops->off &&
	       "led_off: subclass must implement off()");
	return me->ops->off(me);
}

int led_set_brightness(struct led_base *me, uint8_t brightness)
{
	if (!me || !me->ops)
		return -1;
	if (!me->ops->set_brightness) {
		printf("  [%s] no dimming, skip\n", me->name);
		return 0;
	}
	return me->ops->set_brightness(me, brightness);
}

/* ============== GPIO 子类 ============== */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	printf("  [%s] led_on -> platform=%s\n", me->name, platform_name());
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, !self->on_level);
	me->is_on = false;
	return 0;
}

static const struct led_ops gpio_ops = {
	.on  = gpio_on,
	.off = gpio_off,
};

void led_gpio_init(struct led_gpio *me, const char *name,
		   uint8_t pin, bool on_level)
{
	me->base.ops   = &gpio_ops;
	me->base.name  = name;
	me->base.is_on = false;
	me->pin        = pin;
	me->on_level   = on_level;
	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
}

/* ============== PWM 子类 ============== */

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	printf("  [%s] led_on -> platform=%s, pwm ch%u duty=%u%%\n",
	       me->name, platform_name(),
	       (unsigned)self->channel, (unsigned)self->duty);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	printf("  [%s] led_off -> platform=%s, pwm ch%u duty=0%%\n",
	       me->name, platform_name(), (unsigned)self->channel);
	me->is_on = false;
	return 0;
}

static int pwm_set_brightness(struct led_base *me, uint8_t brightness)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	if (brightness > 100)
		brightness = 100;
	self->duty = brightness;
	printf("  [%s] set_brightness -> platform=%s, pwm ch%u duty=%u%%\n",
	       me->name, platform_name(),
	       (unsigned)self->channel, (unsigned)brightness);
	me->is_on = (brightness > 0);
	return 0;
}

static const struct led_ops pwm_ops = {
	.on             = pwm_on,
	.off            = pwm_off,
	.set_brightness = pwm_set_brightness,
};

void led_pwm_init(struct led_pwm *me, const char *name,
		  uint8_t channel, uint8_t duty)
{
	me->base.ops   = &pwm_ops;
	me->base.name  = name;
	me->base.is_on = false;
	me->channel    = channel;
	me->duty       = duty;
}

/* ============== I2C 子类 ============== */

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	printf("  [%s] led_on -> platform=%s, i2c bus%u addr=0x%02X\n",
	       me->name, platform_name(),
	       (unsigned)self->bus, (unsigned)self->addr);
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	printf("  [%s] led_off -> platform=%s, i2c bus%u addr=0x%02X\n",
	       me->name, platform_name(),
	       (unsigned)self->bus, (unsigned)self->addr);
	me->is_on = false;
	return 0;
}

static const struct led_ops i2c_ops = {
	.on  = i2c_on,
	.off = i2c_off,
};

void led_i2c_init(struct led_i2c *me, const char *name,
		  uint8_t bus, uint8_t addr)
{
	me->base.ops   = &i2c_ops;
	me->base.name  = name;
	me->base.is_on = false;
	me->bus        = bus;
	me->addr       = addr;
}
```

### 文件 8：`container_of.h`（11 行）

container_of 宏的最小可用版。和 ch13 / ch14 同源。

```c
/* SPDX-License-Identifier: MIT */
#ifndef MY_CONTAINER_OF_H
#define MY_CONTAINER_OF_H

#include <stddef.h>

#define container_of(ptr, type, member)					\
	((type *)((char *)(ptr) - offsetof(type, member)))

#endif
```

### 文件 9：`platform_ops.h`（44 行）

platform 层内部头文件。`struct platform_ops` 定义、三个实例的 extern 声明、`platform_select` / `platform_name` 切换 + 查询 API。驱动层不 include 这一份，只 platform 层自己 include。

```c
/* SPDX-License-Identifier: MIT */
/*
 * platform_ops.h - platform 层内部 ops 表
 *
 * 这一份是 platform 层的内部细节，对外不暴露给驱动层。驱动层只调
 *   platform_gpio_init / platform_gpio_write / platform_gpio_read
 * 这一组封装函数（在 platform.h 里声明）。
 *
 * 三个 platform_ops 实例（PC 模拟 / STM32 模拟 / Linux 模拟）共存，
 * 启动期或测试期通过 platform_select(&platform_xxx) 切换内部 ops 指针。
 * 封装函数体内部走 ops dispatch，自动落到当前选定的实现。
 *
 * 这是 ch11 之后 platform 层"内部"演化的最终形态。对外接口（封装函数
 * 的签名）从 ch01 起没动过，这是工业代码里"对外稳定、对内可换"的
 * 标准做法。
 */

#ifndef PLATFORM_OPS_H
#define PLATFORM_OPS_H

#include <stdint.h>
#include <stdbool.h>

struct platform_ops {
	const char *name;
	void (*gpio_init)(uint8_t pin, uint8_t mode);
	void (*gpio_write)(uint8_t pin, bool value);
	bool (*gpio_read)(uint8_t pin);
	void (*gpio_deinit)(uint8_t pin);
};

/* 三个具体实例（在各自的实现文件里定义） */
extern const struct platform_ops platform_pc;
extern const struct platform_ops platform_stm32_mock;
extern const struct platform_ops platform_linux_mock;

/* 切换内部 ops 实例 */
void platform_select(const struct platform_ops *p);

/* 当前 platform 名字（封装函数，给日志打印用） */
const char *platform_name(void);

#endif /* PLATFORM_OPS_H */
```

### 文件 10：`platform_dispatch.c`（69 行）

platform 层封装函数实现 + 内部 ops 分发。`g_platform_ops` 是文件作用域的内部指针，外部链接不到。`platform_select` 切换它指向哪个实例。`platform_gpio_xxx` 封装函数内部走 ops dispatch，自动落到当前选定平台。

```c
/* SPDX-License-Identifier: MIT */
/*
 * platform_dispatch.c - platform 层封装函数 + 内部 ops 分发
 *
 * 这个文件做两件事：
 *
 *   1. 实现 common/platform.h 里声明的封装函数：
 *        platform_gpio_init / platform_gpio_write /
 *        platform_gpio_read / platform_gpio_deinit
 *      封装函数体内部走 g_platform_ops->xxx(...) dispatch，自动落到
 *      当前选定的平台实现。
 *
 *   2. 实现 platform_ops.h 里声明的切换 + 查询 API：
 *        platform_select / platform_name
 *
 * 驱动层、应用层永远只调封装函数，看不到 g_platform_ops 这个内部指针。
 * 这是工业代码里的标准做法："对外是普通 C 函数，对内可以是任何实现"。
 */

#include "platform.h"
#include "platform_ops.h"
#include <stdio.h>

/*
 * 内部 ops 指针。文件作用域，外部不可见。默认指向 PC 实例，
 * main.c 启动后通过 platform_select 切换。
 */
static const struct platform_ops *g_platform_ops = &platform_pc;

void platform_select(const struct platform_ops *p)
{
	if (p) {
		g_platform_ops = p;
		printf(">>> platform switched to: %s\n", p->name);
	}
}

const char *platform_name(void)
{
	return g_platform_ops ? g_platform_ops->name : "(unset)";
}

/* ============== 封装函数：内部走 ops dispatch ============== */

void platform_gpio_init(uint8_t pin, uint8_t mode)
{
	if (g_platform_ops && g_platform_ops->gpio_init)
		g_platform_ops->gpio_init(pin, mode);
}

void platform_gpio_write(uint8_t pin, bool value)
{
	if (g_platform_ops && g_platform_ops->gpio_write)
		g_platform_ops->gpio_write(pin, value);
}

bool platform_gpio_read(uint8_t pin)
{
	if (g_platform_ops && g_platform_ops->gpio_read)
		return g_platform_ops->gpio_read(pin);
	return false;
}

void platform_gpio_deinit(uint8_t pin)
{
	if (g_platform_ops && g_platform_ops->gpio_deinit)
		g_platform_ops->gpio_deinit(pin);
}
```

### 文件 11：`platform_ops_pc.c`（42 行）

PC 模拟版 platform_ops 实例。打印动作到屏幕，标记 [PC]。

```c
/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_pc.c - PC 模拟版 platform_ops 实例
 *
 * 这个文件只定义一个 const 实例，不维护任何全局状态。切换实例的逻辑
 * 在 platform_dispatch.c 里。这是 ch15 的"对外封装、对内 ops"分层。
 */

#include "platform_ops.h"
#include <stdio.h>

static void pc_gpio_init(uint8_t pin, uint8_t mode)
{
	const char *s = (mode == 0) ? "OUTPUT" : "INPUT";
	printf("    [PC]    Pin%u init as %s\n", (unsigned)pin, s);
}

static void pc_gpio_write(uint8_t pin, bool value)
{
	printf("    [PC]    Pin%u <- %s\n", (unsigned)pin,
	       value ? "HIGH" : "LOW");
}

static bool pc_gpio_read(uint8_t pin)
{
	(void)pin;
	return false;
}

static void pc_gpio_deinit(uint8_t pin)
{
	printf("    [PC]    Pin%u released\n", (unsigned)pin);
}

const struct platform_ops platform_pc = {
	.name        = "PC",
	.gpio_init   = pc_gpio_init,
	.gpio_write  = pc_gpio_write,
	.gpio_read   = pc_gpio_read,
	.gpio_deinit = pc_gpio_deinit,
};
```

### 文件 12：`platform_ops_stm32_mock.c`（52 行）

STM32 平台的"假装版"。在 PC 上为了演示运行时切换，只打印 BSRR 寄存器写入动作（真机上这一步就是 `GPIOA->BSRR = (1u << pin)`）。

```c
/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_stm32_mock.c - STM32 平台的"假装版"
 *
 * 真正 STM32 上这一份会调 HAL_GPIO_WritePin。在 PC 上为了演示
 * "运行时切换平台"，它只是把动作打到屏幕，标记 [STM32]。
 *
 * 真实 STM32 工程的对应版本见 stm32-snippet/。两份的接口完全一样，
 * 应用层 / led 层 / board 层一字不改。
 */

#include "platform_ops.h"
#include <stdio.h>

static void stm32_gpio_init(uint8_t pin, uint8_t mode)
{
	const char *s = (mode == 0) ? "OUTPUT" : "INPUT";
	printf("    [STM32] Pin%u init as %s (config GPIOA MODER)\n",
	       (unsigned)pin, s);
}

static void stm32_gpio_write(uint8_t pin, bool value)
{
	/*
	 * 真机上这里就是：
	 *   GPIOA->BSRR = value ? (1u << pin) : (1u << (pin + 16));
	 * BSRR 一次 32 位 store，原子。
	 */
	printf("    [STM32] BSRR <- 0x%08X (Pin%u %s)\n",
	       value ? (1u << pin) : (1u << (pin + 16)),
	       (unsigned)pin, value ? "HIGH" : "LOW");
}

static bool stm32_gpio_read(uint8_t pin)
{
	(void)pin;
	return false;
}

static void stm32_gpio_deinit(uint8_t pin)
{
	printf("    [STM32] Pin%u config back to analog\n", (unsigned)pin);
}

const struct platform_ops platform_stm32_mock = {
	.name        = "STM32",
	.gpio_init   = stm32_gpio_init,
	.gpio_write  = stm32_gpio_write,
	.gpio_read   = stm32_gpio_read,
	.gpio_deinit = stm32_gpio_deinit,
};
```

### 文件 13：`platform_ops_linux_mock.c`（48 行）

Linux 用户态"假装版"。在 PC 上打印 sysfs 文件写入动作（真机上这一步就是 `echo 1 > /sys/class/gpio/gpioN/value`）。

```c
/* SPDX-License-Identifier: MIT */
/*
 * platform_ops_linux_mock.c - Linux 用户态"假装版"
 *
 * 真机上这一份会写 /sys/class/gpio/gpioN/value。在 PC 上为了演示
 * "运行时切换平台"，它只是把动作打到屏幕，标记 [LINUX]。
 *
 * 真实 Linux 工程的对应版本见 linux-snippet/。
 */

#include "platform_ops.h"
#include <stdio.h>

static void linux_gpio_init(uint8_t pin, uint8_t mode)
{
	const char *dir = (mode == 0) ? "out" : "in";
	printf("    [LINUX] echo %u > /sys/class/gpio/export\n",
	       (unsigned)pin);
	printf("    [LINUX] echo %s > /sys/class/gpio/gpio%u/direction\n",
	       dir, (unsigned)pin);
}

static void linux_gpio_write(uint8_t pin, bool value)
{
	printf("    [LINUX] echo %d > /sys/class/gpio/gpio%u/value\n",
	       value ? 1 : 0, (unsigned)pin);
}

static bool linux_gpio_read(uint8_t pin)
{
	(void)pin;
	return false;
}

static void linux_gpio_deinit(uint8_t pin)
{
	printf("    [LINUX] echo %u > /sys/class/gpio/unexport\n",
	       (unsigned)pin);
}

const struct platform_ops platform_linux_mock = {
	.name        = "LINUX",
	.gpio_init   = linux_gpio_init,
	.gpio_write  = linux_gpio_write,
	.gpio_read   = linux_gpio_read,
	.gpio_deinit = linux_gpio_deinit,
};
```

### 文件 14：`Makefile`（21 行）

注意 ch15 不再链接 `common/platform_pc.c`。本章 platform 层完全自己实现（`platform_dispatch.c` + 三个 ops 实例文件），编进的就是这一套。`common/platform.h` 还在被 include（提供封装函数声明 + GPIO_MODE_OUTPUT 等常量）。

```makefile
# Makefile - ch15 platform climax (PC)

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -I../../common
TARGET  = demo
SRCS    = main.c app.c led.c board_init.c \
          platform_dispatch.c \
          platform_ops_pc.c platform_ops_stm32_mock.c platform_ops_linux_mock.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe
```

### 跑一遍

```bash
cd oop-in-c/code/15-platform/pc
make
./demo
```

### 期望输出

```
=========================================
  ch15 - platform abstraction climax
=========================================
    [PC]    Pin10 init as OUTPUT

========== run on PC ==========
>>> platform switched to: PC

--- power_on_test ---
    [PC]    Pin10 <- HIGH
  [ERR] led_on -> platform=PC
    [PC]    Pin10 <- LOW
  [STAT] led_on -> platform=PC, pwm ch1 duty=50%
  [STAT] led_off -> platform=PC, pwm ch1 duty=0%
  [NET] led_on -> platform=PC, i2c bus0 addr=0x20
  [NET] led_off -> platform=PC, i2c bus0 addr=0x20

--- alarm_blink ---
    [PC]    Pin10 <- HIGH
  [ERR] led_on -> platform=PC
    [PC]    Pin10 <- LOW

--- status_breathe ---
  [STAT] set_brightness -> platform=PC, pwm ch1 duty=30%
  [STAT] set_brightness -> platform=PC, pwm ch1 duty=80%
  [STAT] set_brightness -> platform=PC, pwm ch1 duty=0%

========== run on STM32 ==========
>>> platform switched to: STM32

--- power_on_test ---
    [STM32] BSRR <- 0x00000400 (Pin10 HIGH)
  [ERR] led_on -> platform=STM32
    [STM32] BSRR <- 0x04000000 (Pin10 LOW)
  [STAT] led_on -> platform=STM32, pwm ch1 duty=0%
  [STAT] led_off -> platform=STM32, pwm ch1 duty=0%
  [NET] led_on -> platform=STM32, i2c bus0 addr=0x20
  [NET] led_off -> platform=STM32, i2c bus0 addr=0x20

--- alarm_blink ---
    [STM32] BSRR <- 0x00000400 (Pin10 HIGH)
  [ERR] led_on -> platform=STM32
    [STM32] BSRR <- 0x04000000 (Pin10 LOW)

--- status_breathe ---
  [STAT] set_brightness -> platform=STM32, pwm ch1 duty=30%
  [STAT] set_brightness -> platform=STM32, pwm ch1 duty=80%
  [STAT] set_brightness -> platform=STM32, pwm ch1 duty=0%

========== run on LINUX ==========
>>> platform switched to: LINUX

--- power_on_test ---
    [LINUX] echo 1 > /sys/class/gpio/gpio10/value
  [ERR] led_on -> platform=LINUX
    [LINUX] echo 0 > /sys/class/gpio/gpio10/value
  [STAT] led_on -> platform=LINUX, pwm ch1 duty=0%
  [STAT] led_off -> platform=LINUX, pwm ch1 duty=0%
  [NET] led_on -> platform=LINUX, i2c bus0 addr=0x20
  [NET] led_off -> platform=LINUX, i2c bus0 addr=0x20

--- alarm_blink ---
    [LINUX] echo 1 > /sys/class/gpio/gpio10/value
  [ERR] led_on -> platform=LINUX
    [LINUX] echo 0 > /sys/class/gpio/gpio10/value

--- status_breathe ---
  [STAT] set_brightness -> platform=LINUX, pwm ch1 duty=30%
  [STAT] set_brightness -> platform=LINUX, pwm ch1 duty=80%
  [STAT] set_brightness -> platform=LINUX, pwm ch1 duty=0%

=========================================
  same app/led/board, 3 platforms
=========================================

Press Enter to exit...
```

## 15.15 跑一遍

```
cd oop-in-c/code/15-platform/pc
make
./demo
```

输出会看到三段：PC、STM32、LINUX，每段跑一次完整业务流。应用层代码 / led 驱动 / board 层完全相同。

## 15.16 视频回放

> [《C 语言·换硬件不改应用｜OOP 完整框架·全系列工具组装》](https://www.bilibili.com/video/BV1Zpo9BxEYG/)

## 一句金句

好的架构不是让你写更多代码，是让你改更少代码。

![金句](../assets/ch15/slide8_金句.png)

## 下一章

LED 框架隔离了主板的变化（同一份应用，不同板型）。Platform 层隔离了芯片的变化（同一份 driver，不同芯片）。

但这一招的威力远不止这两层。Linux 内核就是这一招放大几万倍的产物，所以人们说"Linux 复杂"，实际上是因为不知道它就用了这几招。

下一章揭穿：为什么 Linux 一点都不难。

下一篇：[第 16 章 · 为什么 Linux 一点都不难](16-Linux不难.md)
