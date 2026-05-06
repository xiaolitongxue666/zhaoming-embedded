# 第 13 章 · container_of 的地址魔法 · 向下转型

配套代码：[`oop-in-c/code/13-container-of/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/oop-in-c/code/13-container-of/)

第 12 章解决了向上转型：应用层用 `struct led_base *` 句柄，硬件是谁都不问。

但反过来呢？子类的实现函数 `gpio_on` 收到的也是 `struct led_base *`。它要操作 `pin`，`pin` 在 `struct led_gpio` 里，不在 base 里。怎么从 base 反推回 gpio？

这一章揭穿。

## 13.1 base 里没有 pin

打开第 12 章的 `gpio_on`：

```c
static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	platform_gpio_write(self->pin, self->on_level);
	/* ... */
}
```

第一行用 `(struct led_gpio *)me` 把 base 指针强转回 `struct led_gpio *`。这一行能跑，但藏着一颗雷。

它的前提是：**base 必须是 `struct led_gpio` 的第 0 个字段**。

```c
struct led_gpio {
	struct led_base base;       /* 偏移 0 */
	uint8_t         pin;
	bool            on_level;
};
```

base 在偏移 0，所以 `(struct led_gpio *)me` 直接把 base 的地址当 gpio 的起始地址用，刚好对。

但如果哪天有人为了对齐、为了字段分组、为了塞一个 `magic` 校验字段，把 base 挪到第二个位置：

```c
struct led_gpio {
	uint16_t        magic;      /* 偏移 0 */
	struct led_base base;       /* 偏移 4（含 padding） */
	uint8_t         pin;
	bool            on_level;
};
```

`(struct led_gpio *)me` 算出来的 self 指针就比 gpio 真实地址早 4 字节。`self->pin` 拿到的是 magic 字段的一部分，崩或乱码。编译器一句话不说就让你过了。

![问题：base 里没有 pin](../assets/ch13/slide1_问题展示.png)

## 13.2 偏移量这件事

你需要的不是"假设 base 在第一个"，是"不管 base 在第几个，都对"。

仔细看，问题里有两个已知量：

1. me 的地址（base 字段在内存里的位置）
2. base 字段在 struct led_gpio 里的偏移

未知量：struct led_gpio 的起始地址。

只要把已知 1 减去已知 2，就是未知量。

```
struct led_gpio 起始地址 = base 地址 - base 在 struct 里的偏移
```

![内存布局](../assets/ch13/slide2_内存布局.png)

类比生活：你走在一条路上，看见路牌写"离起点 300 米"。退 300 米，回到起点。

## 13.3 编译期算偏移：offsetof

C 标准库（`stddef.h`）有一个宏专门干这件事：

```c
#include <stddef.h>

size_t off = offsetof(struct led_gpio, base);
```

`offsetof(T, m)` 返回成员 `m` 在结构体 `T` 里的字节偏移。

它怎么知道？编译器知道每个 struct 怎么排（每个字段大小、对齐、padding 都是编译器算出来的），所以这个宏在编译期就能展开成一个常量。常见实现是：

```c
#define offsetof(T, m)    ((size_t)&(((T *)0)->m))
```

假装地址 0 处放了一个 T，看 m 在哪个位置。从 0 数起的距离就是偏移量。整个表达式编译期算完，运行时是一个立即数。

![offsetof](../assets/ch13/slide3_强制转换.png)

## 13.4 把它串起来：container_of

有了 offsetof，把"成员地址减偏移"这一招包成一个宏：

```c
#define container_of(ptr, type, member)             \
	((type *)((char *)(ptr) - offsetof(type, member)))
```

三步：

1. `(char *)(ptr)`：转成字节指针，让减法按字节算（`int *` 减 1 减的是 4 字节）。
2. `- offsetof(type, member)`：减去成员在外层 struct 里的偏移，退回 struct 起点。
3. `(type *)`：把结果按外层 struct 的类型解读。

![container_of 推导](../assets/ch13/slide4_offsetof.png)

`container_of` 不在 C 标准里。它是 Linux 内核自己写的宏，用 offsetof 构造出来。完整版定义在 Linux 内核 v6.6 LTS `include/linux/container_of.h` 第 18-23 行（如何获取内核源码做参考阅读见附录 D）：

```c
#define container_of(ptr, type, member) ({                              \
	void *__mptr = (void *)(ptr);                                   \
	static_assert(__same_type(*(ptr), ((type *)0)->member) ||       \
		      __same_type(*(ptr), void),                        \
		      "pointer type mismatch in container_of()");       \
	((type *)(__mptr - offsetof(type, member))); })
```

内核版用 GNU C 的 statement expression（`({ ... })`）多包了一层。这一节把它每一行展开看。

第一行 `void *__mptr = (void *)(ptr);` 把传进来的 ptr 抓到一个局部 void 指针变量。两个理由：

1. **避免重求值**。如果你写 `container_of(get_next_base(), struct led_gpio, base)`，`get_next_base()` 是函数调用。最小版那个三步宏会把 `(ptr)` 展开两次（一次进 char 强转、一次进 type 强转），等于调两次 `get_next_base()`，第二次返回值可能完全不一样。statement expression 把 ptr 求值一次，存进 `__mptr`，后面的步骤都用 `__mptr` 不再碰 ptr。
2. **统一指针类型**。后面要做减法，`__mptr` 用 void * 类型，让 GCC 把它当字节指针算（GCC 扩展里 `void *` 算术合法），写起来比 `(char *)` 干净。

第二行的 `static_assert + __same_type`：

```c
static_assert(__same_type(*(ptr), ((type *)0)->member) ||
	      __same_type(*(ptr), void),
	      "pointer type mismatch in container_of()");
```

`static_assert` 是 C11 的关键字（C++11 也有），编译期断言。`__same_type` 是 GCC 扩展，比较两个表达式的类型是不是一致。整行字面意思：**编译期检查 `ptr` 指向的类型和 `((type *)0)->member` 的类型一致**。

为什么必要？看个错例：

```c
struct led_gpio {
	struct led_base base;
	uint8_t         pin;
};

struct led_pwm {
	struct led_base base;
	uint8_t         channel;
};

void f(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_pwm, base);
	/* 笔误，写成了 led_pwm */
	self->pin = 99;   /* 但 led_pwm 没有 pin 字段 */
}
```

老的最小版 container_of 这个错要等到链接报错才发现（甚至有时候不会报）。内核版的 `static_assert` 在你写 container_of 这一行就编译失败：`*(ptr)` 是 `struct led_base`，`((struct led_pwm *)0)->base` 也是 `struct led_base`，看起来相同。但如果你写的是 `container_of(some_int_ptr, struct led_gpio, base)`，`*(some_int_ptr)` 是 int，`((struct led_gpio *)0)->base` 是 struct led_base，不同，立刻报 `pointer type mismatch in container_of()`。

把"传错指针"挡在编译期，是这个 static_assert 的核心价值。第二个 OR 条件 `__same_type(*(ptr), void)` 是放行 `void *` 入参（少数代码这么用）。

第三行才是真正的减法：

```c
((type *)(__mptr - offsetof(type, member)));
```

statement expression 的"返回值"是最后一条语句的值，整个 `({ ... })` 求值结果就是这条减法表达式的结果。

PC 上不依赖 GNU C 扩展的最小可用版就是 13.4 节开头那个三步宏，本书配套代码 `pc/container_of.h` 里就是这一份。逻辑等价，编译期约束少一点，PC 上跑没影响。但生产代码里我推荐用内核版，那个 static_assert 能在 commit 前帮你抓住 90% 的低级错。

![container_of](../assets/ch13/slide5_container_of推导.png)

## 13.5 在 gpio_on 里用一下

把第 12 章的 `gpio_on` 改成 container_of 版本：

```c
#include "container_of.h"

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	return 0;
}
```

注意第三个参数 `base`，是成员名，不是变量名。这一行字面意思："给我从 me 出发，找到那个把 me 当作 `base` 字段的 `struct led_gpio` 对象"。

PWM、I2C 子类同套路，每个实现函数第一行都是 container_of。

![gpio_on 用 container_of](../assets/ch13/slide6_gpio_on使用.png)

## 13.6 现在 base 想放哪就放哪

为了证明这一招与位置无关，配套代码故意把 GPIO 子类的 base 挪到第二个位置：

```c
struct led_gpio {
	uint16_t        magic;
	struct led_base base;
	uint8_t         pin;
	bool            on_level;
};
```

跑一下 `./demo`：

```
offsetof(struct led_gpio, base) = 4
offsetof(struct led_pwm,  base) = 0
offsetof(struct led_i2c,  base) = 0
[GPIO] Pin10 init as OUTPUT
...
--- toggle ERR ---
[GPIO] Pin10 -> HIGH
  [ERR] GPIO Pin10 ON (magic=0xCAFE)
[GPIO] Pin10 -> LOW
  [ERR] GPIO Pin10 OFF
```

GPIO 的 base 在偏移 4，但 `gpio_on` 还是正确还原了 `magic = 0xCAFE`、`pin = 10`、`on_level = true`。container_of 和位置无关。

如果把 `container_of` 换回第 12 章的 `(struct led_gpio *)me`，`magic` 立刻变成乱码，`self` 的位置算错了 4 字节。

## 13.7 这个东西叫什么

你刚才做的这件事，拿到一个父类指针，反推回原本的子类对象，计算机科学里叫 **向下转型**（downcasting）。

C++ 的对应物是 `dynamic_cast`：

```cpp
LedBase *base = ...;
LedGpio *gpio = dynamic_cast<LedGpio *>(base);
if (gpio) {
	gpio->pin = ...;
}
```

`dynamic_cast` 干两件事：

1. 编译期检查 `LedBase` 和 `LedGpio` 之间真有继承关系（不然报错）。
2. 运行时查 RTTI（Run-Time Type Information）表，确认 base 指针指向的是不是真的 `LedGpio` 对象，不是就返回 nullptr。

C++ 给你一张安全网。代价是：每个含 virtual 函数的类要带上 type_info 元数据，每次 dynamic_cast 调用都要查表。

container_of 没有安全网，但代价是零：

> container_of 一旦编译完，就是一条减法指令。零运行时开销。

C 用编译期数学解决了 C++ 用运行时类型信息解决的同一个问题。

![C 对比 C++](../assets/ch13/slide7_金句CvsCpp.png)

## 13.8 视频里没讲透的几个细节

### 13.8.1 编译器算偏移的过程

```c
struct led_gpio {
	uint16_t        magic;     /* 偏移 0，2 字节 */
	struct led_base base;      /* 偏移 4 */
	uint8_t         pin;
	bool            on_level;
};
```

为什么 base 不在偏移 2 而在偏移 4？因为 `struct led_base` 第一个成员是 `const struct led_ops *ops`，指针在 32 位机器上对齐 4 字节，所以编译器在 `magic` 后面塞 2 字节 padding，让 base 落到 4 的倍数。

`offsetof(struct led_gpio, base)` 在编译期算出 4，编进 container_of 宏。运行时就是一句 `sub r0, r0, #4`。

### 13.8.2 const 属性会丢

Linux 内核版 container_of 注释里特意写了：

> WARNING: any const qualifier of @ptr is lost.

```c
const struct led_base *me = ...;
struct led_gpio *self = container_of(me, struct led_gpio, base);
self->pin = 99;     /* 编译过，但破坏了 me 的 const 承诺 */
```

container_of 的实现里有一步 `(char *)(ptr)`，强转之后 const 就没了。Linux 内核 6.5 之后引入了 `container_of_const` 用 `_Generic` 保住 const，但老代码到处都在用裸的 container_of。需要严格 const-correctness 时手动检查一下。

### 13.8.3 ptr 表达式不要有副作用

如果你写：

```c
struct led_gpio *self = container_of(get_next_base(), struct led_gpio, base);
```

PC 版那个最小宏是 `(type *)((char *)(ptr) - offsetof(...))`，`ptr` 只出现一次。但内核版用 statement expression，提前把 ptr 抓进 `__mptr` 局部变量再用。如果你的项目 container_of 是宏 + 多次展开 ptr 的版本，`get_next_base()` 会被调多次，行为不可预测。

写 container_of 时尽量用一个已经求值好的指针变量当 ptr。这是 Linux 内核 commit 历史里反复出现的坑。

### 13.8.4 container_of 在内核里出现多少次

整个 Linux 内核源码 `git grep -c container_of` 大概 4-5 万处。从字符设备到网络栈到块层到内存管理子系统，到处都是这一行。

它撑起了 Linux 整套"基类（subsystem 的 struct）+ 子类（驱动 driver 的 struct）"模式。第 16 章会展开 GPIO 子系统，那里 `container_of(desc->gdev->chip, struct gpio_chip_xxx, chip)` 这种用法会反复出现。

### 13.8.5 一句话给自己一个三件套

如果你刚看完 13.4 节觉得头疼，正常。我当年第一次看 container_of 也琢磨了一段时间。

不需要现在就完全理解每一步。记住三件事就够：

1. **入：成员指针 + 外层 struct 类型 + 成员名**。
2. **出：外层 struct 的指针**。
3. **代价：编译期算偏移，运行时一条减法**。

后面写代码用几次，自然就懂了。

## 13.9 你现在的代码在 STM32 上长什么样

container_of 在 STM32 上是同一个宏，编译产物就是 ARM Cortex-M 的 `SUB Rd, Rn, #imm`。STM32 工程里你直接 `#include <linux/types.h>` 不行（那是内核头），但 STM32CubeIDE 的 `arm-none-eabi-gcc` 自带 `<stddef.h>`，offsetof 一直可用。把 13.4 节的三步宏放进 utility 头文件就能用。

完整 STM32 snippet 见 [`oop-in-c/code/13-container-of/stm32-snippet/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/oop-in-c/code/13-container-of/stm32-snippet/)。

## 13.10 你现在的代码在 Linux 用户态长什么样

Linux 用户态可以直接借 `<stddef.h>` 的 offsetof。绝大多数 user-space 项目自定义自己的 container_of 宏，比如 systemd / gobject / libnl 都各写一份，逻辑都一样。在 Linux 用户态写 OOP 风格 C 代码，container_of 这一招几乎是入场券。

完整 Linux 用户态 snippet 见 [`oop-in-c/code/13-container-of/linux-snippet/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/oop-in-c/code/13-container-of/linux-snippet/)。

## 13.11 工业代码里的 container_of 长什么样

工业控制板 driver 里：

```c
/* drivers/encoder/encoder.h */
struct encoder_base;

struct encoder_ops {
	int (*read)(struct encoder_base *me, int32_t *count);
	int (*reset)(struct encoder_base *me);
};

struct encoder_base {
	const struct encoder_ops *ops;
	const char               *name;
};

/* drivers/encoder/encoder_quad.c */
struct encoder_quad {
	struct encoder_base base;
	int32_t             count;
	uint8_t             a_pin;
	uint8_t             b_pin;
};

static int quad_read(struct encoder_base *me, int32_t *count)
{
	struct encoder_quad *self = container_of(me, struct encoder_quad, base);
	*count = self->count;
	return 0;
}
```

整个工业项目里，10 多个 driver 模块，每个子类实现函数第一行都是 container_of。这一行已经稳定到所有同事看一眼就知道意思，不需要解释。

读完这一章你拿到内核源码、读到 driver 源码、读到 GObject 源码，第一行的 container_of 不会再让你头疼。

## 13.12 完整源码清单

把下面的代码块分别保存到对应的文件，目录结构和 [`oop-in-c/code/13-container-of/pc/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/oop-in-c/code/13-container-of/pc/) 一致。`make && ./demo` 即可跑通。

`common/platform.h` 和 `common/platform_pc.c` 跟 ch01 一字不变，本章 Makefile 通过 `-I../../common` 引用。完整代码见 ch01 末尾完整源码清单。

### 文件 1：`main.c`（60 行）

应用层。三个子类对象 `g_err / g_stat / g_net` 在 main 里声明，循环 toggle，故意把 GPIO 子类的 base 放到第二个位置，证明 container_of 与位置无关。

```c
/* SPDX-License-Identifier: MIT */
/*
 * main.c - container_of 演示
 *
 * GPIO 子类故意把 base 放到第二个位置，证明 container_of 与位置无关。
 * 强转 (struct led_gpio *)me 在这种布局下会算错，container_of 一直对。
 */

#include "led.h"
#include "container_of.h"
#include <stddef.h>
#include <stdio.h>

static struct led_gpio g_err   = {0};
static struct led_pwm  g_stat  = {0};
static struct led_i2c  g_net   = {0};

int main(void)
{
	printf("=========================================\n");
	printf("  ch13 - container_of\n");
	printf("=========================================\n");

	/* 让大家看一眼偏移量。GPIO 的 base 故意不在 0 位 */
	printf("offsetof(struct led_gpio, base) = %u\n",
	       (unsigned)offsetof(struct led_gpio, base));
	printf("offsetof(struct led_pwm,  base) = %u\n",
	       (unsigned)offsetof(struct led_pwm, base));
	printf("offsetof(struct led_i2c,  base) = %u\n",
	       (unsigned)offsetof(struct led_i2c, base));

	led_gpio_init(&g_err,  "ERR",  10, true);
	led_pwm_init (&g_stat, "STAT",  1, 50);
	led_i2c_init (&g_net,  "NET",   0, 0x20);

	struct led_base *handles[] = {
		&g_err.base,
		&g_stat.base,
		&g_net.base,
	};

	for (int i = 0; i < 3; i++) {
		printf("\n--- toggle %s ---\n", handles[i]->name);
		led_on(handles[i]);
		led_off(handles[i]);
	}

	printf("\n--- breath stat ---\n");
	led_set_brightness(handles[1], 60);
	led_set_brightness(handles[1], 0);

	printf("\n=========================================\n");
	printf("  base offset != 0 still works\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
```

### 文件 2：`container_of.h`（30 行）

container_of 宏的最小可用版（PC 友好版）。三步：转字节指针、减偏移、转回外层 struct 指针。Linux 内核版多了 `__same_type` 编译期类型检查，最小版去掉这一层方便 PC 上任何 C99 编译器跑通。

```c
/* SPDX-License-Identifier: MIT */
/*
 * container_of.h - Linux 内核 container_of 宏的最小可用版本
 *
 * 完整版见 Linux 内核 include/linux/container_of.h，本文件保留同样的
 * 三步语义、去掉 GCC 专属的类型检查（用 static_assert），便于 PC 上
 * 用任何 C99 编译器跑通。
 */

#ifndef MY_CONTAINER_OF_H
#define MY_CONTAINER_OF_H

#include <stddef.h>     /* offsetof */

/**
 * container_of - 从成员指针反推外层 struct 起始地址
 * @ptr:    指向某个成员的指针
 * @type:   外层 struct 的类型
 * @member: 该成员在外层 struct 里的名字
 *
 * 三步：
 *   1. 把 ptr 转成字节指针（char *），让减法按字节算
 *   2. 减去 member 在 type 里的偏移
 *   3. 把结果转回 type *
 */
#define container_of(ptr, type, member)					\
	((type *)((char *)(ptr) - offsetof(type, member)))

#endif /* MY_CONTAINER_OF_H */
```

### 文件 3：`led.h`（66 行）

LED 父类 + 三个子类的声明。和 ch12 的差别：GPIO 子类前面塞了一个 `uint16_t magic` 字段，让 base 故意不在偏移 0。container_of 还能反推回去。

```c
/* SPDX-License-Identifier: MIT */
/*
 * led.h - led_base + 子类（ch13 版）
 *
 * 和 ch12 唯一的差别：子类实现里用 container_of 反推自己。
 * base 不一定要在第一个位置，本章故意把 base 挪到中间，证明
 * container_of 与位置无关。
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

/* GPIO 子类：base 故意不放第一个位置 */
struct led_gpio {
	uint16_t        magic;     /* 故意挡在前面，让 base 的偏移不为 0 */
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

### 文件 4：`led.c`（163 行，和 ch12 比改了 6 处）

父类统一接口和 ch12 一字不变。子类实现里六处把 `(struct led_xxx *)me` 强转换成 `container_of(me, struct led_xxx, base)`。GPIO 子类 init 里多了一行 `me->magic = 0xCAFE` 用于演示。

```c
/* SPDX-License-Identifier: MIT */
/*
 * led.c - 子类实现里用 container_of 反推自己
 *
 * 和 ch12 的差别只有一处：(struct led_xxx *)me 强转换成
 *   container_of(me, struct led_xxx, base);
 *
 * GPIO 子类的 base 故意不在第一个位置，container_of 照样对。
 */

#include "led.h"
#include "container_of.h"
#include "platform.h"
#include <assert.h>
#include <stdio.h>

/* ============== 父类统一接口 ============== */

int led_on(struct led_base *me)
{
	if (!me)
		return -1;
	/* on 是 LED 的核心能力，子类必须实现。调试构建里 assert 抓到
	 * 忘填的子类立刻 abort，Release 构建定义 NDEBUG 后 assert 整行
	 * 消失，0 运行时开销。 */
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
	/* set_brightness 是选填字段，子类没实现就走父类默认行为
	 * （这里默认 = 安静返回 0）。 */
	if (!me->ops->set_brightness)
		return 0;
	return me->ops->set_brightness(me, brightness);
}

/* ============== 子类一：GPIO LED ============== */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	printf("  [%s] GPIO Pin%u ON (magic=0x%04X)\n",
	       me->name, (unsigned)self->pin, (unsigned)self->magic);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, !self->on_level);
	me->is_on = false;
	printf("  [%s] GPIO Pin%u OFF\n", me->name, (unsigned)self->pin);
	return 0;
}

static const struct led_ops gpio_ops = {
	.on  = gpio_on,
	.off = gpio_off,
};

void led_gpio_init(struct led_gpio *me, const char *name,
		   uint8_t pin, bool on_level)
{
	me->magic       = 0xCAFE;
	me->base.ops    = &gpio_ops;
	me->base.name   = name;
	me->base.is_on  = false;
	me->pin         = pin;
	me->on_level    = on_level;

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, !on_level);
}

/* ============== 子类二：PWM LED ============== */

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	printf("  [%s] PWM ch%u duty=%u%%\n",
	       me->name, (unsigned)self->channel, (unsigned)self->duty);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	printf("  [%s] PWM ch%u duty=0%%\n",
	       me->name, (unsigned)self->channel);
	me->is_on = false;
	return 0;
}

static int pwm_set_brightness(struct led_base *me, uint8_t brightness)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	if (brightness > 100)
		brightness = 100;
	self->duty = brightness;
	printf("  [%s] PWM ch%u duty=%u%%\n",
	       me->name, (unsigned)self->channel, (unsigned)brightness);
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

/* ============== 子类三：I2C LED ============== */

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	printf("  [%s] I2C bus%u addr=0x%02X reg=0x01\n",
	       me->name, (unsigned)self->bus, (unsigned)self->addr);
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	printf("  [%s] I2C bus%u addr=0x%02X reg=0x00\n",
	       me->name, (unsigned)self->bus, (unsigned)self->addr);
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

### 文件 5：`Makefile`（19 行）

```makefile
# Makefile - ch13 container_of (PC)

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -I../../common
TARGET  = demo
SRCS    = main.c led.c ../../common/platform_pc.c

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
cd oop-in-c/code/13-container-of/pc
make
./demo
```

### 期望输出

```
=========================================
  ch13 - container_of
=========================================
offsetof(struct led_gpio, base) = 4
offsetof(struct led_pwm,  base) = 0
offsetof(struct led_i2c,  base) = 0
[GPIO] Pin10 init as OUTPUT
[GPIO] Pin10 -> LOW (OFF)

--- toggle ERR ---
[GPIO] Pin10 -> HIGH (ON)
  [ERR] GPIO Pin10 ON (magic=0xCAFE)
[GPIO] Pin10 -> LOW (OFF)
  [ERR] GPIO Pin10 OFF

--- toggle STAT ---
  [STAT] PWM ch1 duty=50%
  [STAT] PWM ch1 duty=0%

--- toggle NET ---
  [NET] I2C bus0 addr=0x20 reg=0x01
  [NET] I2C bus0 addr=0x20 reg=0x00

--- breath stat ---
  [STAT] PWM ch1 duty=60%
  [STAT] PWM ch1 duty=0%

=========================================
  base offset != 0 still works
=========================================

Press Enter to exit...
```

## 13.13 跑一遍

```
cd oop-in-c/code/13-container-of/pc
make
./demo
```

输出节选：

```
offsetof(struct led_gpio, base) = 4
offsetof(struct led_pwm,  base) = 0
offsetof(struct led_i2c,  base) = 0
[GPIO] Pin10 init as OUTPUT

--- toggle ERR ---
[GPIO] Pin10 -> HIGH
  [ERR] GPIO Pin10 ON (magic=0xCAFE)
```

`magic = 0xCAFE` 这一行证明 container_of 准确还原了原始 `struct led_gpio` 对象。

## 13.14 视频回放

> [《C 语言·向下转型｜container_of·从成员反推 struct》](https://www.bilibili.com/video/BV1LEo4B5EZu/)

## 下一章

类型机制完整了：上转下转都能干。

但还有一个洞：如果某种 LED 的 ops 表里 `on` 没填呢？应用层调 `led_on(handle)` 内部走到 `me->ops->on(me)`，NULL 被当函数地址跳过去，程序直接崩。

下一章：虚函数不实现会怎样。

下一篇：[第 14 章 · 虚函数不实现会怎样 · 纯虚与抽象类](14-纯虚与抽象类.md)
