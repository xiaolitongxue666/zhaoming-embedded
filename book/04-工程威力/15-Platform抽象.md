# 第 15 章 · 换硬件不改应用 · OOP 完整框架

配套代码：[`oop-in-c/code/15-platform/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/oop-in-c/code/15-platform/)

封装、继承、多态、向上转型、向下转型、纯虚 / 选填 / 接口，C 里做 OOP 的全部武器你都见过了。

这一章不引入任何新概念。把武器全部组装起来，演示一套完整的 LED 框架：父类 / 子类 / 板级 / 应用四层，每层一个职责，每层只调下一层。同一份应用代码挂着 GPIO + PWM + I2C 三种硬件混搭的 LED，应用层 grep 拿不到任何硬件字样。换硬件方案，应用 0 修改。

最初你一份代码控一盏灯、三盏灯三份代码。现在 300 多行复制粘贴的代码，被压到应用层 60 行。一路走来，从一团乱麻到一套架构。

## 15.1 四层架构

打开配套代码 `oop-in-c/code/15-platform/pc/`，8 个文件按调用方向从上往下分四层：

```
应用层    app.h, app.c          alarm_blink / status_indicate / power_on_test
子类层    led.c (子类那部分)    led_gpio / led_pwm / led_i2c, container_of 反推
父类层    led.h, led.c (父类)   led_base + led_ops + 必填 / 选填分发
板级层    leds.h, board_init.c  实例化 + 向上转型, 唯一认识硬件的文件
```

底下还有一份 `common/platform_pc.c`，提供 4 个 GPIO 封装函数（`platform_gpio_init / write / read / deinit`），从 ch01 起整本书一字不变。STM32 / Linux 真机上换成对应实现，上面 4 层一字不动。

每一层只关心自己。每一层只调下一层。

![文件结构](../assets/ch15/slide1_文件结构.png)

## 15.2 父类层：led_base + 必填选填

`led.h` 里定型整本书最终版的 `led_base`：

```c
struct led_ops {
	int (*on)(struct led_base *me);              /* 必填 */
	int (*off)(struct led_base *me);             /* 必填 */
	int (*set_brightness)(struct led_base *me,   /* 选填 */
			      uint8_t brightness);
};

struct led_base {
	const struct led_ops *ops;
	const char           *name;
	bool                  is_on;
};
```

三个字段：`name`（每盏灯一个字符串名字，打日志用）、`is_on`（当前开关状态，父类记账）、`ops`（指向子类的 ops 表）。这三个字段从 ch11 多态那一章定型之后再没动过。

`led.c` 的父类统一接口分两种处理：

```c
int led_on(struct led_base *me)
{
	if (!me)
		return -1;
	/* 必填: 子类必须实现 on. assert 抓到忘填的子类立刻 abort 给行号. */
	assert(me->ops && me->ops->on &&
	       "led_on: subclass must implement on()");
	return me->ops->on(me);
}

int led_set_brightness(struct led_base *me, uint8_t b)
{
	if (!me || !me->ops)
		return -1;
	if (!me->ops->set_brightness) {     /* 选填, 父类提供默认行为 */
		printf("  [%s] no dimming, skip (brightness=%u)\n",
		       me->name, (unsigned)b);
		return 0;
	}
	return me->ops->set_brightness(me, b);
}
```

`on / off` 必填，对应 C++ 纯虚函数，子类不实现 `assert` 立刻报错。`set_brightness` 选填，对应 C++ 带默认行为的虚函数，子类不实现父类走默认（GPIO 灯没法调光）。同一个父类接口里，必填和选填两种做法同时出现。这就是 ch14 的精确兑现。

![父类代码](../assets/ch15/slide2_父类代码.png)

## 15.3 子类层：三种硬件

子类把 `led_base` 嵌进自己的结构体（哪个位置都行，配套代码 ch13 已经证明 `container_of` 不挑位置）：

```c
struct led_gpio {
	struct led_base base;
	uint8_t         pin;
	bool            on_level;       /* 0=低电平亮, 1=高电平亮 */
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

每个子类的实现函数第一行都是 `container_of`，从 base 反推回子类对象：

```c
static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	return 0;
}
```

反推成功之后，就能访问 `self->pin` 和 `self->on_level` 这些子类自己的字段。`platform_gpio_write` 是 ch01 起就存在的封装函数，子类不直接碰寄存器，调下层一个普通 C 函数。

```c
static const struct led_ops gpio_ops = { .on = gpio_on, .off = gpio_off };

static const struct led_ops pwm_ops = {
	.on             = pwm_on,
	.off            = pwm_off,
	.set_brightness = pwm_set_brightness,
};

static const struct led_ops i2c_ops = { .on = i2c_on, .off = i2c_off };
```

GPIO 和 I2C 子类只填 `on / off`，`set_brightness` 留空（C 标准里静态存储未显式初始化的字段会被零初始化为 NULL，父类的选填默认行为会接住）。PWM 子类三件套全填。

最后是参数化的子类构造函数：

```c
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
```

四个参数从外面传进来：对象指针、名字、引脚号、点亮电平。硬件资源不在子类定义里写死。这是 C 模拟构造函数的标准做法。

`led_pwm.c`、`led_i2c.c` 同套路：自己的字段、自己的 ops、自己的 init 构造函数。换一种硬件，再写一个子类文件，父类 / 板级 / 应用 一字不动。

![子类代码](../assets/ch15/slide3_子类代码.png)

## 15.4 板级层：唯一认识硬件的文件

`leds.h` 对外只暴露三个全局句柄 + `board_init` 入口：

```c
#ifndef LEDS_H
#define LEDS_H

#include "led.h"

extern struct led_base *g_led_error;
extern struct led_base *g_led_status;
extern struct led_base *g_led_network;

void board_init(void);

#endif
```

句柄类型是父类指针 `struct led_base *`，不是具体子类。应用层 `#include "leds.h"` 拿到的就是这一组句柄，分不清底下挂的是 GPIO 还是 PWM 还是 I2C。

`board_init.c` 是整个工程里**唯一**认识硬件的文件：

```c
static struct led_gpio s_led_err;       /* 文件作用域, 外部不可见 */
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

报警灯用 GPIO 开关，最简单；状态灯用 PWM 呼吸，要能调亮度；网络灯挂在 I2C 扩展芯片上，是远程芯片上的灯。三种不同硬件，通过同一个 `struct led_base *` 接口对外暴露。

实例化三行：三个子类对象都是空的，没赋任何值。硬件资源全部 `init` 的时候传进去。pin 编号、PWM 通道、I2C 地址这些常量集中在这一个文件里。

最后三行绑定：每个子类对象取 `&xxx.base` 拿到 base 字段地址，赋给对应的全局父类指针句柄。这一步就是把子类对象"当作"父类指针在用。**向上转型**（ch12 § 12.2）：因为 `base` 是子类的字段，C99 § 6.7.2.1 保证结构体字段的地址等于结构体加上偏移量后的地址，编译器替我们算偏移（base 在第一个字段时偏移是 0，一条 `ADD r0, #0` 就被优化掉了）。

绑定完成后，全局句柄准备好了。应用层 `#include "leds.h"`，随便用。

![板级代码](../assets/ch15/slide4_板级代码.png)

## 15.5 应用层：grep 零硬件字样

`app.c` 三个真实业务函数：

```c
#include "leds.h"
#include "app.h"
#include <stdio.h>

void alarm_blink(void)
{
	led_on(g_led_error);
	led_off(g_led_error);
}

void status_indicate(int err_code)
{
	if (err_code == 0)
		led_on(g_led_status);
	else
		led_on(g_led_error);
}

void power_on_test(void)
{
	led_on(g_led_error);    led_off(g_led_error);
	led_on(g_led_status);   led_off(g_led_status);
	led_on(g_led_network);  led_off(g_led_network);
}
```

不是一个 `test_led` 就完了，是三个业务函数：报警闪烁（一开一关）、状态指示（按错误码挑亮哪盏）、开机自检（三盏灯依次亮一遍）。十几处调用，全部只用 `g_led_*` 全局句柄。

不信？打开终端：

```
$ grep -nE "led_gpio|led_pwm|led_i2c"  app.c
$ grep -nE "gpio_write|HAL_GPIO|sysfs"  app.c
$ grep -nE "BSRR|0x[0-9A-F]"            app.c
```

三条 grep 全部 0 命中。板级混搭了 GPIO、PWM、I2C 三种硬件，应用层一个都不认识。它只看到三个 `struct led_base *` 句柄，调 `led_on / led_off / led_set_brightness`。剩下的事，是哪种子类、走哪个 ops、哪个引脚、哪个总线，全部不关心。

![应用层 + grep](../assets/ch15/slide5_应用层grep.png)

## 15.6 换硬件 diff：board_init 改三行，app.c 零改动

真实场景：周五下午六点，老板进来。客户改要求：报警灯要能调光，从 GPIO 换成 PWM。

打开 `board_init.c`，改 3 行：

```diff
-static struct led_gpio s_led_err;
+static struct led_pwm  s_led_err;
 static struct led_pwm  s_led_status;
 static struct led_i2c  s_led_net;

 void board_init(void)
 {
-    led_gpio_init(&s_led_err, "ERR", 10, true);
+    led_pwm_init (&s_led_err, "ERR",  2, 80);
     led_pwm_init (&s_led_status, "STAT", 1, 50);
     led_i2c_init (&s_led_net,    "NET",  0, 0x20);

     g_led_error   = &s_led_err.base;
     g_led_status  = &s_led_status.base;
     g_led_network = &s_led_net.base;
 }
```

类型那行：`struct led_gpio` 改成 `struct led_pwm`。
init 那行：`led_gpio_init` 换成 `led_pwm_init`，参数从"10 号引脚、高电平点亮"换成"2 号通道、80% 亮"。
绑定那行：`&s_led_err.base` 一字不动，`base` 这个字段名两个子类都有，绑定逻辑天然兼容。

三行改动全部在 `board_init.c` 里面。`app.c`、`led.c`、`leds.h` 全部 0 改动。`alarm_blink`、`status_indicate`、`power_on_test` 三个业务函数一行不动。

周五晚上回家吃饭。

老板很高兴，顺手又塞了两个新需求过来。

![换硬件 diff](../assets/ch15/slide6_换硬件diff.png)

## 15.7 Before / After：300 行 → 60 行

来看你走了多远。

**最初**（ch01 - ch04）：

```c
/* gpio_led.c, gpio_led_2.c, gpio_led_3.c — 三份独立代码 */
void red_led_on(void)    { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_SET);   }
void red_led_off(void)   { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_RESET); }
void green_led_on(void)  { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,  GPIO_PIN_SET);   }
void green_led_off(void) { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,  GPIO_PIN_RESET); }
/* ... 8 颗 LED 乘 4 个函数 = 32 个几乎一模一样的函数, 300 多行 */
```

应用层和 HAL 库直接耦合。改一个名字格式要改三个文件。换硬件？认真考虑辞职。

**现在**（ch15）：

```c
/* app.c */
led_on(g_led_error);
led_set_brightness(g_led_status, 80);
```

四层架构：父类定义接口、子类实现接口、板级绑定硬件、应用层只用句柄。应用层 60 行。换硬件方案改 `board_init.c` 三行，应用 0 修改。

代码从 300 多行压到 60 行。

但**真正重要**的不是行数少，是改一处全生效。

一路走来，从一团乱麻到一套架构。

你学的不是语法，是管理复杂系统的思维方式。

![Before / After](../assets/ch15/slide7_BeforeAfter.png)

## 15.8 一句金句

> 好的架构不是让你写更多代码，是让你改更少代码。

![金句](../assets/ch15/slide8_金句.png)

## 15.9 视频里没讲透的几个细节

### 15.9.1 应用层只 include leds.h 不 include led_gpio.h 的纪律

`app.c` 顶上你看到的是：

```c
#include "leds.h"
```

不是：

```c
#include "led.h"          /* 不行: 暴露 struct led_gpio / led_pwm / led_i2c */
#include "led_gpio.h"     /* 不行: 应用层就知道有 GPIO 这种东西了 */
```

`leds.h` 里只 include `led.h`（拿 `struct led_base` 和父类接口声明），完全不暴露子类类型。这条纪律保证应用层 grep 不到任何硬件字样。

工业项目里这一招会做得更彻底：连 `led.h` 都不让应用层看到，应用层只 include `leds.h`，而 `leds.h` 里把 `struct led_base` 做成不完整类型（forward declaration），只允许指针操作：

```c
/* leds.h - 工业版 */
struct led_base;     /* 不完整类型, 应用层只能用指针 */
extern struct led_base *g_led_error;
int led_on(struct led_base *me);
```

应用层连 `struct led_base` 长什么样都不知道，更别提 `struct led_gpio`。本书早期章节为了演示方便没做到这么彻底，真实工业代码里"应用层连父类内部都看不见"是常态。

### 15.9.2 全局句柄 vs static + getter 的取舍

ch15 用的是一组 `extern struct led_base *g_led_xxx` 全局句柄。简单直接，一行 `extern` 就能让应用层拿到。

工业项目里另一种常见做法是 `static + getter`：

```c
/* board_init.c */
static struct led_base *s_led_error;       /* static, 只这个文件可见 */

struct led_base *led_error_get(void)        /* 暴露 getter */
{
	return s_led_error;
}
```

应用层调 `led_error_get()` 而不是直接读 `g_led_error`。代价是多一层函数调用，好处是：

- 可以加 lazy init（第一次调用时才初始化）
- 可以加访问日志 / 锁 / 引用计数
- 可以在 getter 里做"未初始化检查"

教学版用 extern 全局句柄因为最直观；工业版常见 getter，因为接口更稳定（句柄实现可换）。两种都见过你就够了。

### 15.9.3 板级 mix-and-match：GPIO + PWM + I2C 同时混搭的可读性

ch15 `board_init.c` 同时实例化了 GPIO 灯、PWM 灯、I2C 灯。这是**故意的**。

很多教学项目只敢做一种：要么全 GPIO，要么全 PWM。讲分层架构的书就栽在这里：读者看到三盏 LED 三个 `struct led_gpio`，会以为"分层只在同种硬件之间分层"。

混搭演示的是另一件事：分层的真正威力是**应用层不知道也不必知道每盏灯具体是哪种硬件**。`alarm_blink` 调 `led_on(g_led_error)`，背后是 GPIO 拉高电平、PWM 设占空比、还是 I2C 发包，上层一字不知。

工业项目里这种混搭是常态：一块板子上几十路输出，有的是 MCU 自己 GPIO，有的接 PWM 控制器，有的挂在 I2C 扩展芯片上。`board_init.c` 把这些差异全部吸收掉，应用层只见统一的 `led_base * `句柄。

### 15.9.4 把"换硬件 diff"思路推广到 motor / sensor

LED 这一招完全可以复用到其他外设。Motor：

```c
struct motor_ops {
	int (*set_speed)(struct motor *me, int rpm);
	int (*get_position)(struct motor *me, int32_t *pos);
	int (*stop)(struct motor *me);
};

struct motor {
	const struct motor_ops *ops;
	const char             *name;
};

/* 子类 */
struct motor_pwm { struct motor base; uint8_t pwm_ch; };       /* 直流 PWM 调速 */
struct motor_can { struct motor base; uint8_t can_id; };       /* 总线伺服 */
```

Sensor 同理：温度传感器、压力传感器、IMU，每种都是一种子类。应用层调 `sensor_read(handle, &val)`，背后是 ADC、是 I2C、是 SPI，无所谓。

整本书 ch07 - ch14 学到的所有武器，每种外设都套得进去。LED 是教学线索，但这套思维方式是通用的。ch20 工业实战会再用一次：温度传感器 + 压力传感器 + 流量计混搭在同一个采集任务里。

## 15.10 你现在的代码在 STM32 上长什么样

`stm32-snippet/led_stm32.c` 是真实硬件版的 4 个 platform 封装函数：

```c
#include "platform.h"
#include "stm32f4xx_hal.h"

void platform_gpio_init(uint8_t pin, uint8_t mode)
{
	GPIO_InitTypeDef cfg = {0};
	__HAL_RCC_GPIOA_CLK_ENABLE();
	cfg.Pin   = (uint16_t)(1U << pin);
	cfg.Mode  = (mode == GPIO_MODE_OUTPUT) ?
	            GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
	cfg.Pull  = GPIO_NOPULL;
	cfg.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &cfg);
}

void platform_gpio_write(uint8_t pin, bool value)
{
	HAL_GPIO_WritePin(GPIOA, (uint16_t)(1U << pin),
	                  value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
```

子类里 `platform_gpio_write(self->pin, self->on_level)` 在 STM32 上调到底就是 `HAL_GPIO_WritePin → GPIOx->BSRR = (1u << pin)`，一次 32 位 store，原子。

应用层 `app.c` / 父类 `led.c` / 板级 `board_init.c` 一字不改。封装函数 `platform_gpio_write` 的签名一字不改。换的就是这一份 `led_stm32.c`。

## 15.11 你现在的代码在 Linux 用户态长什么样

`linux-snippet/led_linux.c` 走 sysfs 文件读写：

```c
void platform_gpio_write(uint8_t pin, bool value)
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
```

子类 `gpio_on` 调 `platform_gpio_write(self->pin, self->on_level)`，最终走到 `echo 1 > /sys/class/gpio/gpioN/value`，触发内核 gpiolib 操作物理引脚。更现代的做法是 libgpiod，附录 C 会展开。

同样，应用层 / 父类 / 板级一字不改。

## 15.12 工业代码里的"换硬件 diff"长什么样

工业项目里这一章的精神已经渗透到每个层级。挑一个真实场景：报警子系统。

需求：板子上有 8 路报警输出。客户 A 用 LED 灯提示，客户 B 接蜂鸣器，客户 C 接外接报警柱（24V 信号）。三家客户买的是同一个主控板，只有"报警这一路"硬件不同。

工业代码里 `board_init.c` 长这样：

```c
/* board_init_customerA.c - 8 路全 LED */
static struct led_gpio s_alarm[8];

void board_init(void)
{
	led_gpio_init(&s_alarm[0], "ALM_0", 10, true);
	/* ... 7 个 ... */
	for (int i = 0; i < 8; i++)
		g_alarm[i] = &s_alarm[i].base;
}
```

```c
/* board_init_customerB.c - 8 路全蜂鸣器 (PWM 控频率) */
static struct led_pwm s_alarm[8];

void board_init(void)
{
	led_pwm_init(&s_alarm[0], "ALM_0", 1, 50);
	/* ... 7 个 ... */
	for (int i = 0; i < 8; i++)
		g_alarm[i] = &s_alarm[i].base;
}
```

```c
/* board_init_customerC.c - 8 路全外接报警柱 (I2C 扩展芯片) */
static struct led_i2c s_alarm[8];

void board_init(void)
{
	led_i2c_init(&s_alarm[0], "ALM_0", 0, 0x20);
	/* ... 7 个 ... */
	for (int i = 0; i < 8; i++)
		g_alarm[i] = &s_alarm[i].base;
}
```

三家客户三份 `board_init_*.c`，编译期挑一份链进去。应用层报警逻辑（`alarm_trigger / alarm_clear / alarm_run_self_test`）一份代码三家共用。这就是"产品角色 → 硬件映射"的工程化形态：业务代码完全不知道客户 A B C 之间的硬件差异，硬件差异全部锁在 `board_init` 这一个文件里。

ch19 / ch20 工业实战会展开真实主控板项目里的板级文件，看看一份 `board_init.c` 怎么管 30+ 路硬件。

## 15.13 完整源码清单 + 跑一遍

把下面的代码块分别保存到对应文件，目录结构和 [`oop-in-c/code/15-platform/pc/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/oop-in-c/code/15-platform/pc/) 一致。`make && ./demo` 即可跑通。

8 个文件：

```
pc/
├── main.c             主程序入口
├── app.h, app.c       应用层 - 三个业务函数
├── leds.h             板级对外暴露的 g_led_xxx 句柄声明
├── board_init.c       板级 - 唯一认识硬件的文件
├── led.h, led.c       父类 + 三个子类
├── container_of.h     与 ch13 同款 (最小可用版)
└── Makefile           链接 ../../common/platform_pc.c
```

### 文件 1：`main.c`

```c
/* SPDX-License-Identifier: MIT */
#include "app.h"
#include "leds.h"
#include <stdio.h>

int main(void)
{
	printf("=========================================\n");
	printf("  ch15 - OOP complete framework demo\n");
	printf("=========================================\n");

	board_init();

	power_on_test();
	alarm_blink();
	status_indicate(0);   /* 正常 -> 状态灯 */
	status_indicate(1);   /* 故障 -> 报警灯 */

	printf("\n=========================================\n");
	printf("  app.c never named any hardware type\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
```

### 文件 2：`app.h`

```c
/* SPDX-License-Identifier: MIT */
#ifndef APP_H
#define APP_H

void alarm_blink(void);
void status_indicate(int err_code);
void power_on_test(void);

#endif
```

### 文件 3：`app.c`

```c
/* SPDX-License-Identifier: MIT */
#include "leds.h"
#include "app.h"
#include <stdio.h>

void alarm_blink(void)
{
	printf("\n--- alarm_blink ---\n");
	led_on(g_led_error);
	led_off(g_led_error);
}

void status_indicate(int err_code)
{
	printf("\n--- status_indicate(err_code=%d) ---\n", err_code);
	if (err_code == 0)
		led_on(g_led_status);
	else
		led_on(g_led_error);
}

void power_on_test(void)
{
	printf("\n--- power_on_test ---\n");
	led_on(g_led_error);    led_off(g_led_error);
	led_on(g_led_status);   led_off(g_led_status);
	led_on(g_led_network);  led_off(g_led_network);
}
```

### 文件 4：`leds.h`

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

### 文件 5：`board_init.c`

```c
/* SPDX-License-Identifier: MIT */
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

### 文件 6：`led.h`

```c
/* SPDX-License-Identifier: MIT */
#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdbool.h>

struct led_base;

struct led_ops {
	int (*on)(struct led_base *me);                 /* 必填 */
	int (*off)(struct led_base *me);                /* 必填 */
	int (*set_brightness)(struct led_base *me,      /* 选填 */
			      uint8_t brightness);
};

struct led_base {
	const struct led_ops *ops;
	const char           *name;
	bool                  is_on;
};

int led_on(struct led_base *me);
int led_off(struct led_base *me);
int led_set_brightness(struct led_base *me, uint8_t brightness);

struct led_gpio {
	struct led_base base;
	uint8_t         pin;
	bool            on_level;
};
void led_gpio_init(struct led_gpio *me, const char *name,
		   uint8_t pin, bool on_level);

struct led_pwm {
	struct led_base base;
	uint8_t         channel;
	uint8_t         duty;
};
void led_pwm_init(struct led_pwm *me, const char *name,
		  uint8_t channel, uint8_t duty);

struct led_i2c {
	struct led_base base;
	uint8_t         bus;
	uint8_t         addr;
};
void led_i2c_init(struct led_i2c *me, const char *name,
		  uint8_t bus, uint8_t addr);

#endif
```

### 文件 7：`led.c`

```c
/* SPDX-License-Identifier: MIT */
#include "led.h"
#include "container_of.h"
#include "platform.h"
#include <assert.h>
#include <stdio.h>

/* 父类统一接口 */

int led_on(struct led_base *me)
{
	if (!me)
		return -1;
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

int led_set_brightness(struct led_base *me, uint8_t b)
{
	if (!me || !me->ops)
		return -1;
	if (!me->ops->set_brightness) {
		printf("  [%s] no dimming, skip (brightness=%u)\n",
		       me->name, (unsigned)b);
		return 0;
	}
	return me->ops->set_brightness(me, b);
}

/* GPIO 子类 */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	printf("  [%s] led_on  -> GPIO Pin%u\n",
	       me->name, (unsigned)self->pin);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, !self->on_level);
	me->is_on = false;
	printf("  [%s] led_off -> GPIO Pin%u\n",
	       me->name, (unsigned)self->pin);
	return 0;
}

static const struct led_ops gpio_ops = {
	.on = gpio_on, .off = gpio_off,
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

/* PWM 子类 (三件套全填) + I2C 子类 (只填 on/off): 见配套源码. */
```

### 文件 8：`Makefile`

```makefile
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -I../../common
TARGET  = demo
SRCS    = main.c app.c led.c board_init.c ../../common/platform_pc.c

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

```
cd oop-in-c/code/15-platform/pc
make
./demo
```

期望输出（节选）：

```
=========================================
  ch15 - OOP complete framework demo
=========================================
[GPIO] Pin10 init as OUTPUT

--- power_on_test ---
[GPIO] Pin10 -> HIGH (ON)
  [ERR] led_on  -> GPIO Pin10
[GPIO] Pin10 -> LOW (OFF)
  [ERR] led_off -> GPIO Pin10
  [STAT] led_on  -> PWM ch1 duty=50%
  [STAT] led_off -> PWM ch1 duty=0%
  [NET] led_on  -> I2C bus0 addr=0x20
  [NET] led_off -> I2C bus0 addr=0x20

--- alarm_blink ---
[GPIO] Pin10 -> HIGH (ON)
  [ERR] led_on  -> GPIO Pin10
[GPIO] Pin10 -> LOW (OFF)
  [ERR] led_off -> GPIO Pin10

--- status_indicate(err_code=0) ---
  [STAT] led_on  -> PWM ch1 duty=50%

--- status_indicate(err_code=1) ---
[GPIO] Pin10 -> HIGH (ON)
  [ERR] led_on  -> GPIO Pin10

=========================================
  app.c never named any hardware type
=========================================
```

`[ERR] led_on  -> GPIO Pin10` 这一行打的就是子类层在做的事：同一句 `led_on(g_led_error)`，对 ERR 落到 GPIO 拉高电平、对 STAT 落到 PWM 设占空比、对 NET 落到 I2C 发包。应用层一字不知。

## 15.14 视频回放

> [《C 语言·换硬件不改应用｜OOP 完整框架·全系列工具组装》](https://www.bilibili.com/video/BV1Zpo9BxEYG/)

## 下一章

你的框架完整了。但它只隔离了主板的变化（同一份应用，三种 LED 硬件混搭）。

子类里这一行 `platform_gpio_write(self->pin, self->on_level)` 落到底，还是 STM32 的 BSRR、Linux 的 sysfs、瑞萨的 DR 寄存器。硬件名字一变，这一行就要重写。

主板的变化，LED 层隔离了。芯片的变化，谁来隔离？

下一章揭穿。同一招用第二次。

下一篇：[第 16 章 · 为什么 Linux 一点都不难](16-Linux不难.md)
