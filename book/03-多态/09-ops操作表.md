# 第 9 章 · 参数长到换行

配套代码：[`oop-in-c/code/09-ops-table/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/oop-in-c/code/09-ops-table/)

## 9.1 一个真实场景

第 8 章你把 `on / off` 函数指针当参数传给 `test_led`。两个参数还行。

但需求来了。一颗 LED 你不光要测开关，还要测翻转、调亮度。`test_led` 的参数列表跟着膨胀：

```c
void test_led(struct led_base *me,
              void (*on)(struct led_base *),              /* 1 */
              void (*off)(struct led_base *),             /* 2 */
              void (*toggle)(struct led_base *),          /* 3 */
              void (*set_brightness)(struct led_base *, int)); /* 4 */
```

4 个函数指针参数。声明换两次行才写得下。调用更长：

```c
test_led(&red_led.base,
         gpio_on_pull_high,
         gpio_off,
         gpio_toggle,
         gpio_set_brightness);
```

调用一次写五行。读着累、写着累。

更要命的：`on / off / toggle` 这几个函数指针的类型一模一样，都是 `void (*)(struct led_base *)`。如果调用方手抖把 `on` 和 `off` 顺序传反了：

```c
test_led(&red_led.base,
         gpio_off,              /* 这里本来该传 on */
         gpio_on_pull_high,     /* 这里本来该传 off */
         gpio_toggle,
         gpio_set_brightness);
```

编译器看不出来。类型对就放行。运行起来，开灯调到了 off 函数，关灯调到了 on 函数。该亮的时候灭，该灭的时候亮。这种 bug 能查一天。

散装的电话号码摊了一桌子，纸条多了，拿错一张都不知道。

![参数列表爆炸](../assets/ch09/slide1_参数列表爆炸.png)

## 9.2 typedef 给函数指针起短名字

第一步先解决"类型声明又臭又长"这件事。

```c
void (*on)(struct led_base *me);
void (*off)(struct led_base *me);
void (*toggle)(struct led_base *me);
void (*set_brightness)(struct led_base *me, int val);
```

每个函数指针的类型字面量都要写一坨 `void (*)(struct led_base *)`。写一次能看懂，写第三次看着累。

C 语言里有个工具叫 `typedef`，给类型起一个别名：

```c
typedef void (*led_on_fn)(struct led_base *me);
typedef void (*led_off_fn)(struct led_base *me);
typedef void (*led_action_fn)(struct led_base *me);              /* 通用一元 action */
typedef void (*led_brightness_fn)(struct led_base *me, int val);
```

这之后 `led_on_fn fp;` 等价于 `void (*fp)(struct led_base *me);`，但读起来短一截。和你早就在用的 `typedef unsigned int uint32_t` 是同一个语法、同一个用途：给一个长得难读的类型起一个短名字。

`typedef` 在 ch01 1.7.3 节讲 `typedef struct` 时提过，Linus 反对的是那种"藏类型信息"的滥用（写 `typedef struct foo {} foo_t;` 让你看 `foo_t` 不知道是 struct）。**函数指针 typedef 是少数 Linus 也支持的 typedef 例外**：原始类型字面量太长，起短名是纯收益。Linux 内核的 `struct file_operations` 字段也是这么 typedef 一遍的。

注意 `typedef` 本身不分配存储、不生成代码、不引入新类型，只是给已有类型起别名。编译完字节码里看不出有过 typedef。

![typedef 给函数指针起短名](../assets/ch09/slide2_typedef简化.png)

## 9.3 装进盒子：struct led_ops

名字起好了，下一步把这一组函数指针打包进一个 `struct`。

观察一件事：`on / off / toggle` 是一组绑死的东西。它们一起描述了"一种 LED 的所有行为"。GPIO 风格 LED 这一组，PWM 风格 LED 那一组。一组绑死的东西打包，C 里的工具就是 struct：

```c
struct led_ops {
    led_action_fn on;
    led_action_fn off;
    led_action_fn toggle;
};
```

一个 struct，三个函数指针字段，每个字段一个名字。这就是**操作表**（ops table）。

`test_led` 现在接 ops 指针：

```c
int test_led(struct led_base *me, const struct led_ops *ops)
{
    if (!me || !ops || !ops->on || !ops->off || !ops->toggle)
        return -1;

    ops->on(me);
    ops->toggle(me);
    ops->off(me);
    return 0;
}
```

调用方填好一张表，传进去：

```c
const struct led_ops led_ops_gpio = {
    .on     = gpio_on_pull_high,
    .off    = gpio_off,
    .toggle = gpio_toggle,
};

test_led(&red_led.base, &led_ops_gpio);
```

参数列表从 5 个塞到了 2 个。`test_led` 内部按名字访问：`ops->on` 永远是 on，`ops->off` 永远是 off，不可能传反。编译器在初始化 `led_ops_gpio` 时帮你查 `.on` 这个字段类型对不对（不对就编译报错）。

散装的函数指针装订成一本电话簿。散装号码丢了找不到，电话簿翻开就有，一个不少。

**注意 ops 表里只装"做法不同"的行为**。`on / off / toggle / set_brightness` 这种每种 LED 实现都不一样的（拉电平 vs 配占空比 vs 写 I2C 命令），才需要做成函数指针让运行时绑定。`get_name`、`get_state` 这种只是读父类字段的函数不用进 ops 表，沿用 ch06 引入的 `led_get_name(&xxx.base)` 直接读 base 数据就行。**做法不同的进 ops 表，读数据的不进**。这个判断准则下一章会反复用到。

本章为了和后续章节衔接，ops 表只放最核心的三件套 `on / off / toggle`。`set_brightness` 留给 9.5.2 节当"选填字段"的例子。

ops 表通常加 `const` 修饰：

```c
const struct led_ops led_ops_gpio = { ... };
```

`const` 三层意义：

1. 不允许运行时改动 ops 表内容。所有同类型 LED 共享同一张表，改了灾难
2. 链接时进 `.rodata` 段。Flash 上有备份，RAM 不占空间
3. 下一章 `base->ops` 字段也写 `const struct led_ops *ops`，把"我手里这个 ops 表别动"的意图传到字段上

`extern` 暴露给用户，调用方拿 `&led_ops_gpio` 就是 ops 表的地址：

```c
/* led.h */
extern const struct led_ops led_ops_gpio;
extern const struct led_ops led_ops_pwm;
```

![装进盒子·struct led_ops](../assets/ch09/slide3_LedOps_struct.png)

## 9.4 这个东西叫什么

把一组相关的函数指针打包进一个 struct，让别人通过表名按名访问，软件工程里有个名字。

它叫**操作表**（ops table），也叫**虚函数表**（vtable）。Linux 内核源码里把这种 struct 都叫 `xxx_ops`：`file_operations`、`net_device_ops`、`gpio_chip` 里的回调集，都是 ops 表。

C++ 里你写：

```cpp
class led_base {
public:
    virtual void on() = 0;
    virtual void off() = 0;
    virtual void toggle() = 0;
};

class led_gpio : public led_base { ... };
class led_pwm  : public led_base { ... };
```

带 `virtual` 函数的 class，C++ 编译器在背后**做三件事**：

1. **生成一张函数指针表**。每个 class 一张。`led_gpio` 的表里 `.on` 指向 `led_gpio::on`，`led_pwm` 的表里 `.on` 指向 `led_pwm::on`。这张表 C++ 叫 vtable
2. **在每个对象里偷偷加一个指针**，指向这张表（叫 vptr）
3. **调用时自动通过 vptr 查表**找到正确的函数

你这一章亲手做了**第一步**：手写了 `struct led_ops` 这张表。下一章手动做第二步：在 `struct led_base` 里加一个 `const struct led_ops *ops` 字段。第 11 章手动做第三步：通过 `me->ops->on(me)` 找到对的函数调它。

C++ 编译器自动做的事，你 C 里手动做完。底层机器码几乎一字不差。Stroustrup 那句"零成本抽象"的具体形态，就是你这一章手写的那张表。

![C 对比 C++ · 编译器三件事](../assets/ch09/slide4_CvsCpp.png)

视频的金句版总结：**结构化不是束缚，是让混乱变得可管理**。散装号码会丢，电话簿不会。

## 9.5 视频里没讲透的几个细节

### 9.5.1 designated initializer 是 C99 的礼物

```c
const struct led_ops led_ops_gpio = {
    .on     = gpio_on_pull_high,
    .off    = gpio_off,
    .toggle = gpio_toggle,
};
```

这种 `.field_name = value` 的写法叫 designated initializer，C99 引入。好处：

1. **不依赖字段顺序**。哪天 struct 里调换字段顺序，已有 ops 表代码不用改
2. **可读性好**。不用数到第几个字段
3. **未列出的字段自动初始化为 0 / NULL**（C99 标准 6.7.8 节第 21 段）

C89 没有这个语法，只能按字段顺序填：

```c
const struct led_ops led_ops_gpio = {
    gpio_on_pull_high,    /* on */
    gpio_off,             /* off */
    gpio_toggle,          /* toggle */
};
```

字段顺序变了就全乱。Linux 内核早期代码很多这种"按位置 init"，后期重构都改成了 designated initializer。本书一律用 designated 写法。

### 9.5.2 ops 表里某些字段不填怎么办

`set_brightness` 这种字段就是个例子：GPIO LED 硬件上没有调光能力，PWM LED 才有。如果 ops struct 里加上 `set_brightness` 字段：

```c
struct led_ops {
    led_action_fn     on;
    led_action_fn     off;
    led_action_fn     toggle;
    led_brightness_fn set_brightness;     /* 选填 */
};

const struct led_ops led_ops_gpio = {
    .on     = gpio_on_pull_high,
    .off    = gpio_off,
    .toggle = gpio_toggle,
    /* set_brightness 不填, designated initializer 自动置 NULL */
};
```

调用方就得做 NULL check：

```c
if (ops->set_brightness)
    ops->set_brightness(me, 50);
else
    printf("This LED doesn't support brightness control\n");
```

**第 14 章会专门讲这件事的三种处理策略**：报错、空 stub、纯虚函数（约定子类必须填）。本章先用 NULL check 这种最朴素的版本。

### 9.5.3 ops 表的内存代价：共享 vs 每实例一份

```c
struct led_ops {
    led_action_fn on;       /* 4 bytes 32-bit */
    led_action_fn off;
    led_action_fn toggle;
};
```

ARM Cortex-M4 上 3 个函数指针 3 × 4 = 12 字节。在 `.rodata` 段，全程序就一份。

100 颗 LED 的两种实现路线对比：

| 方案 | RAM/Flash 占用 | 灵活性 |
|---|---|---|
| ops 表共享（推荐）：每个对象只存一个 `const struct led_ops *` 指针 | 一张 ops 表 12 字节（Flash），每颗 LED 多 4 字节指针 = 100 × 4 + 12 = **412 字节** | 同类型 LED 必须用同一张表 |
| ops 表每实例一份：每个对象嵌入完整的 `struct led_ops` | 100 × 12 = **1200 字节**（必须在 RAM，因为对象本身在 RAM） | 每颗 LED 可以独立调实现 |

工业代码里的硬规则：**ops 表共享，对象只存 ops 指针**。三条理由：

1. **省 RAM**。100 颗 LED 省 788 字节。MCU 上 RAM 是几十 KB 量级，这点省下来很关键
2. **进 Flash**。共享版 ops 表 `const` 之后链接进 .rodata，烧到 Flash 上零 RAM 占用
3. **缓存友好**。所有同类 LED 调 `ops->on` 都访问同一段地址，CPU 数据缓存命中率几乎 100%；每实例一份会让缓存反复换行，在 Cortex-A53 跑 Linux 时差距明显

下一章 ops 字段就是 `const struct led_ops *ops`，4 字节，所有同类型 LED 共享同一张表。这就是 C++ vtable 的存储策略：**类一份，对象只存 vptr**。

`const` 限制只读这件事还有一个边界：如果你想运行时换实现（比如开发板上跑某段诊断代码时临时把 `.on` 替换成"先打 log 再调原 on"），不能改 const ops 表本身。工业代码 99% 走的是**预先准备多张 ops 表，运行时换 `me->ops` 指针**指向不同表（ch15 platform 切换是这个用法的高潮）。极少数场景才会去掉 const 让 ops 落 .data 允许字段改写。

### 9.5.4 ops 表的初始化顺序问题

C 标准保证：全局 / 静态对象的常量初始化在 main() 之前完成。`const struct led_ops led_ops_gpio = { ... };` 这种形式由编译器在编译期填好，写进 .rodata。

但是！如果 ops 表里某个字段引用了**非常量地址**（比如不是函数地址，而是某个全局函数指针变量），就会进入"运行时动态初始化"，时机取决于实现。

工业代码硬规则：**ops 表只填函数地址，不填变量地址**。函数地址是常量（链接期定）。本章符合。

### 9.5.5 调用 ops->on 的汇编代价

```c
ops->on(me);
```

ARM Cortex-M4 编译出来：

```
LDR  r3, [r0, #0]    ; r0 = ops, 取 ops 表第 0 个字段 = on 的地址 (3 cycle)
BLX  r3              ; 间接跳转 (3 cycle)
```

和上一章独立函数指针变量的调用开销一样：一次 LDR + 一次 BLX，约 28 ns @ 168 MHz。从 ops 表第 0 个字段读 on 地址走的是编译期常量偏移，没有额外的运行时计算。

C++ virtual 函数调用就是这两条指令。Stroustrup 那句"零成本抽象"在 vcall 这里要打个小折扣（间接跳转编译器没法 inline，CPU branch predictor 命中率也低），但这是工程上完全可接受的代价。

### 9.5.6 为什么 ops 不写在 led_base 里

按理说 `struct led_ops` 应该是基类的概念，所有子类共用一份接口定义。本章直接写在 led.h 里，对应"GPIO 风格 LED"和"PWM 风格 LED"两组实现。下一章 ops 才会真正放进 base。

为什么分两章？因为本章先把"打包"这个动作单独讲清楚（ops 表是什么、怎么填、怎么调）。下一章再处理"ops 表跟谁绑、谁带"（放进 base 让对象自带）。一次只解决一件事，认知负荷小。

工业代码里 ops 表的最终落点是 base struct 里的字段，下一章会演化到位。

## 9.6 你现在的代码在 STM32 上长什么样

STM32 端胶水还是 ch01 那套。`led_base.h / led_base.c / led_gpio.h / led_gpio.c / main.c` 一字不改。

ops 表 `led_ops_gpio / led_ops_pwm` 编译后进 `.rodata` 段，烧到 Flash 上。运行时常驻。所有 GPIO 类 LED 共享同一份 12 字节的 ops 表：

```
/* 真实芯片 .map 文件里能看到 */
.rodata
  ...
  led_ops_gpio        0x08001234  12
  led_ops_pwm         0x08001240  12
```

100 颗 LED 仅有 24 字节 ops 表代价，加上每颗 LED 自己的 struct 实例（在 .bss 或栈上）。

本节用的还是函数式包装的 platform 抽象层，是教学简化版。第 11 章后会改成 ops 表式。

## 9.7 你现在的代码在 Linux 用户态长什么样

Linux 端的 ops 表完全一样的写法。`gcc / clang` 把 `const struct` 放到 `.rodata`，进程加载时在虚拟内存里只读映射，全进程共享。

`led.h / led.c / main.c` 一字不改。同 9.6 节 STM32 端，平台层是教学简化版，第 11 章后会演化成 ops 表式。

## 9.8 工业代码里的 ops 表

工业控制板项目里，每个驱动都有一张 ops 表。LED 驱动这样：

```c
/* drivers/led/led.h */
struct led_base;

struct led_ops {
    int (*on)(struct led_base *me);
    int (*off)(struct led_base *me);
    int (*toggle)(struct led_base *me);
};

/* drivers/led/led_gpio.c */
const struct led_ops led_ops_gpio = {
    .on     = led_gpio_on,
    .off    = led_gpio_off,
    .toggle = led_gpio_toggle,
};
```

注意 ops 表里函数指针的参数是 `struct led_base *`，不是某个具体子类。这是因为 ops 表要被基类层使用（下一章讲 ops 放进 base），所有子类的 ops 表必须类型一致。

EEPROM、风扇、蜂鸣器、按键这些 driver 的 ops 表结构各不相同，但写法都是这一套：定义 ops struct，每种实现 fill 一张 const ops 表，应用层通过 base 指针拿到 ops 指针调用。

这就是 Linux 内核 `struct file_operations` 的 OOP 骨架。你这一章亲手推了一遍。

## 9.9 跑一遍

```bash
cd oop-in-c/code/09-ops-table/pc
make
./demo
```

输出节选：

```
========================================
  ops table: pack action pointers as struct.
  test_led takes one ops pointer.
========================================

--- test_led(&red_led.base, &led_ops_gpio) ---
  [test] open ...
[GPIO] Pin13 -> HIGH (ON)
  [GPIO] "red" ON
  [test] toggle ...
[GPIO] Pin13 -> LOW (OFF)
  [GPIO] "red" OFF
  [test] close ...

--- test_led(&blue_led.base, &led_ops_pwm) ---
  [test] open ...
  [PWM] "blue" ON  (channel 1, duty=70)
  [test] toggle ...
  [PWM] "blue" OFF (channel 1)
  [test] close ...
  [PWM] "blue" OFF (channel 1)
```

`test_led` 函数体没改。换一张 ops 表，跑出完全不同的行为。

完整源码见 [`oop-in-c/code/09-ops-table/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/oop-in-c/code/09-ops-table/)。

## 9.10 视频回放

想听口播版的可以看 B 站这一期视频：

> [《C 语言·ops 操作表｜参数长到换行·虚函数表》](https://www.bilibili.com/video/BV1iLdrBUEvf/)

![金句](../assets/ch09/slide5_金句.png)

视频里把这一章叫做"散装电话号码 → 电话簿"。散装函数指针绑成一组共享名字的"号码本"，按名取号永远不会取错。

视频金句：**结构化不是束缚，是让混乱变得可管理**。散装号码会丢，电话簿不会。

## 下一章

ops 表手里捏着，调用方还得**主动**把它传进去。每次都要：

```c
test_led(&red_led, &led_ops_gpio);   /* 调用方记得传 */
test_led(&blue_led, &led_ops_pwm);
```

应用层要记住每颗 LED 该用哪张 ops 表。一旦传错，又是 bug。

能不能让每颗 LED **自己带着**自己的 ops 表？应用层只传 LED 自己，ops 表 LED 自己知道。

下一篇：[第 10 章 · ops 放进对象](10-ops放进对象.md)
