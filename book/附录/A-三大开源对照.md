# 附录 A · 三大开源项目对照（Linux 内核 / Zephyr / GObject）

读到这里你已经看完了从 ch01 一颗 LED 到 ch20 工业控制板 24 虚方法电机的完整演化路径。所有的 OOP 抽象都在工业项目真实代码里跑过。

但你可能还有一个怀疑：这套"struct + me + ops 表 + container_of + initcall"的写法，会不会只是某家中国公司的内部规范？或者只是工业控制板项目的偶然选择？

不是。这套写法是世界标准模式。

这一附录把同一个 OOP 模式放到全球三个最有影响力的 C 开源项目里横向对照：

- **Linux 内核**：4000 万行 C 代码，地球上最大的工程项目
- **Zephyr RTOS**：Linux 基金会托管的开源 RTOS，覆盖几百种 MCU
- **GObject**：GTK 桌面环境与 GNOME 生态的对象系统，C 语言写的运行时

三者语境不同，但抽象骨架同构。你看完这一附录会建立起一种眼光：以后看任何 C 大型项目，几分钟就能找到它的"struct + ops 表"骨架。

## A.1 同一个问题：用 C 写出可扩展的设备抽象

三个项目都要解决一个共同问题：**怎么让 100 种不同的设备共用同一份上层代码**？

Linux 内核要让 100 种字符设备都能 `read()` / `write()`，但每种设备底下读写的实现完全不同。Zephyr 要让 100 种 MCU 的 GPIO 都能 `gpio_pin_set()` 调用，但每颗 MCU 的寄存器布局完全不同。GTK 要让 100 种 GUI 控件都能响应"点击事件"，但每种控件的渲染、布局、事件分发完全不同。

它们的解法**完全一致**：把"差异"塞进一个 ops 表（虚函数表），把"统一接口"放在基类，应用层通过基类指针调用，运行时穿过 vptr 跳到具体实现。

下面具体看每个项目怎么做。

## A.2 Linux 内核的 file_operations

Linux 内核里所有"可以被打开 / 读 / 写 / 关闭"的对象（普通文件、设备节点、pipe、socket、proc 节点 ...）都通过同一个抽象 `struct file_operations` 表达自己。

节选自 Linux 内核 v6.6 `include/linux/fs.h` 第 1852 行起（如何获取内核源码见附录 D）：

```c
struct file_operations {
	struct module *owner;
	loff_t (*llseek) (struct file *, loff_t, int);
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
	ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
	ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
	int (*iopoll)(struct kiocb *kiocb, struct io_comp_batch *,
	              unsigned int flags);
	int (*iterate_shared) (struct file *, struct dir_context *);
	__poll_t (*poll) (struct file *, struct poll_table_struct *);
	long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
	long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
	int (*mmap) (struct file *, struct vm_area_struct *);
	int (*open) (struct inode *, struct file *);
	int (*release) (struct inode *, struct file *);
	int (*fsync) (struct file *, loff_t, loff_t, int datasync);
	/* ... 后面还有十几个 ... */
};
```

完整的 `file_operations` 有 30 多个函数指针。这就是这本书 ch20 你看到的 24 虚方法 ops 表的内核同款，只是规模更大。

具体一个字符设备驱动会这样写自己的 ops 表：

```c
static const struct file_operations my_dev_fops = {
	.owner   = THIS_MODULE,
	.open    = my_dev_open,
	.read    = my_dev_read,
	.write   = my_dev_write,
	.release = my_dev_release,
};
```

`static const` ops 全局唯一一份，所有具体设备实例共享。完全是这本书 ch10 / ch11 的写法。

应用层调用 `read(fd, buf, size)` 经过 system call 层转到 `vfs_read(file, ...)`，最终是：

```c
file->f_op->read(file, buf, size, ppos);
```

`f_op` 就是 `struct file_operations *`，跟你熟悉的 `me->ops->read(me, ...)` 一模一样，只是 Linux 把 `me` 叫 `file`。

注意 `__user` 这个标记。它告诉编译器（实际是 Sparse 静态分析器）"这个指针指向的是用户空间地址，不能直接解引用，必须走 `copy_from_user / copy_to_user`"。这是 Linux 内核为了内核态 / 用户态隔离专门加的注解，工业代码里没有这一层（裸机或 RTOS 没有用户态）。本质还是把 `void *` 加了一个语义标签。

### A.2.1 Linux 的 container_of 宏

第 13 章讲了 `container_of` 宏。现在看它在内核里的定义。节选自 Linux 内核 `include/linux/container_of.h` 第 18 行：

```c
#define container_of(ptr, type, member) ({              \
	void *__mptr = (void *)(ptr);                       \
	static_assert(__same_type(*(ptr), ((type *)0)->member) ||  \
	              __same_type(*(ptr), void),            \
	              "pointer type mismatch in container_of()"); \
	((type *)(__mptr - offsetof(type, member))); })
```

13.x 节推过的 `((type *)((char *)ptr - offsetof(type, member)))` 就是这个宏的核心。Linux 内核里加了 `static_assert` 类型检查，让你在编译期就发现"你给错了 member 名字"这种错误，但本质 100% 一致。

`((` 开头的双括号块是 GCC 的 statement expression 扩展（不是 C 标准），可以让一组语句作为表达式返回最后一个值。这让 `container_of` 既能做类型检查（中间的 `static_assert` 语句），又能返回一个指针表达式。C 标准里宏只能返回一个表达式，没法插入语句；GCC 扩展打破了这个限制，让宏的能力接近内联函数。

`container_of` 在 Linux 内核里到处都是。在内核源码树里 `grep -rn "container_of"` 能找到几万次调用。每一次"基类指针拿回子类指针"都要走它。

最近的内核（5.x 之后）还提供了 `container_of_const`（同文件第 32 行），用 `_Generic` 关键字根据 `ptr` 的 const 性自动选择返回 `const T *` 还是 `T *`，更精确地保留类型限定符。这是 C11 的能力。这一层精致教学版可以忽略，知道存在就行。

### A.2.2 Linux GPIO 子系统：和工业项目一模一样

为了让你看清"同构"，再看一个具体子系统。Linux 内核 GPIO 抽象，节选自 `include/linux/gpio/driver.h` 第 415 行起：

```c
struct gpio_chip {
	const char           *label;
	struct gpio_device   *gpiodev;
	struct device        *parent;
	struct fwnode_handle *fwnode;
	struct module        *owner;

	int  (*request)(struct gpio_chip *gc, unsigned int offset);
	void (*free)(struct gpio_chip *gc, unsigned int offset);
	int  (*get_direction)(struct gpio_chip *gc, unsigned int offset);
	int  (*direction_input)(struct gpio_chip *gc, unsigned int offset);
	int  (*direction_output)(struct gpio_chip *gc, unsigned int offset,
	                         int value);
	int  (*get)(struct gpio_chip *gc, unsigned int offset);
	int  (*get_multiple)(struct gpio_chip *gc, unsigned long *mask,
	                     unsigned long *bits);
	void (*set)(struct gpio_chip *gc, unsigned int offset, int value);
	void (*set_multiple)(struct gpio_chip *gc, unsigned long *mask,
	                     unsigned long *bits);
	int  (*set_config)(struct gpio_chip *gc, unsigned int offset,
	                   unsigned long config);
	int  (*to_irq)(struct gpio_chip *gc, unsigned int offset);
	/* ... 后面还有几十个字段（中断处理、调试输出、引脚有效性掩码等）... */

	int     base;
	u16     ngpio;
	u16     offset;
	const char *const *names;
	bool    can_sleep;
};
```

这个 `struct gpio_chip` 就是工业项目里 `platform_uart_dev_t` 的 Linux 版本：基类一份，每颗 GPIO 控制器（STM32 GPIO、i.MX GPIO、树莓派 BCM283x GPIO ...）派生出自己的实现实例，挂上自己的 `request / direction_output / set / get`。

注意基类字段顺序：先是 `label / gpiodev / parent / fwnode / owner`（公共元数据），然后是一大串函数指针（虚方法），最后是 `base / ngpio / offset / names / can_sleep`（公共数据成员）。这是 Linux 内核里大型基类的标准布局：metadata + ops + data。工业项目里的 `platform_device + ops + private` 也是这个结构。

### A.2.3 GPIO 控制器的注册：gpiochip_add_data

具体一颗 GPIO 控制器把自己注册进系统的方式（节选自 `drivers/gpio/gpiolib.c`，第 737 行）：

```c
int gpiochip_add_data_with_key(struct gpio_chip *gc, void *data,
                               struct lock_class_key *lock_key,
                               struct lock_class_key *request_key)
{
	struct gpio_device *gdev;
	int base = 0;
	int ret = 0;

	gdev = kzalloc(sizeof(*gdev), GFP_KERNEL);
	if (!gdev)
		return -ENOMEM;
	gdev->dev.bus = &gpio_bus_type;
	gdev->dev.parent = gc->parent;
	gdev->chip = gc;

	gc->gpiodev = gdev;
	gpiochip_set_data(gc, data);

	/* ... 引脚号分配、driver model 挂载、debugfs 暴露、IRQ 配置 ... */

	return 0;
}
EXPORT_SYMBOL_GPL(gpiochip_add_data_with_key);
```

每一颗 GPIO 控制器的驱动（`drivers/gpio/gpio-stm32.c`、`gpio-bcm-kona.c`、`gpio-mvebu.c` 等）在自己的 probe 函数里调一次 `gpiochip_add_data(...)`，把自己挂进 GPIO 子系统。挂好之后这颗芯片所有的引脚就能被上层（其它驱动 / 用户空间 sysfs）按 offset 访问。

这跟工业项目里 `platform_uart_register("uart3", ...)` 是同一个机制：一个具体硬件实例把自己的"句柄 + ops 表"注册到全局表里，应用层按名字 / 编号查找。骨架一样，只是 Linux 多了 driver model（device / bus / driver 三件套）+ 引用计数 + sysfs 暴露这几层包装。

应用层（其它驱动）调用：

```c
gpiod_set_value(desc, 1);
```

底下穿过几层抽象最终到 `gc->set(gc, offset, value)`，跟这本书 ch20 工业项目里 `me->ops->velocity_set(me, 200)` **同构**。

### A.2.4 Linux 的 initcall 机制

第 17 章讲过的 initcall，看内核里的定义。节选自 Linux 内核 `include/linux/init.h`：

```c
typedef int (*initcall_t)(void);

#define core_initcall(fn)         __define_initcall(fn, 1)
#define postcore_initcall(fn)     __define_initcall(fn, 2)
#define arch_initcall(fn)         __define_initcall(fn, 3)
#define subsys_initcall(fn)       __define_initcall(fn, 4)
#define fs_initcall(fn)           __define_initcall(fn, 5)
#define device_initcall(fn)       __define_initcall(fn, 6)
#define late_initcall(fn)         __define_initcall(fn, 7)
```

7 级排序，跟工业项目的 INIT_BOARD_EXPORT / INIT_PREV_EXPORT / INIT_DEVICE_EXPORT / ... 一一对应。

具体宏展开：

```c
#define __define_initcall(fn, id) \
	___define_initcall(fn, id, .initcall##id)
```

把函数指针放进 ELF 的 `.initcall1` / `.initcall2` / ... 段。系统启动时遍历这些段依次调用：

```c
/* init/main.c 里 */
static initcall_entry_t *initcall_levels[] = {
	__initcall0_start,
	__initcall1_start,
	__initcall2_start,
	__initcall3_start,
	/* ... */
};

static void do_initcall_level(unsigned int level, char *command_line)
{
	initcall_entry_t *fn;
	for (fn = initcall_levels[level]; fn < initcall_levels[level+1]; fn++)
		do_one_initcall(initcall_from_entry(fn));
}
```

这就是 Linux 内核 4000 万行代码里成千上万个 `module_init(my_driver_init)` 的全部秘密。工业项目的 INIT_xxx_EXPORT 宏一字不差地复刻这个机制（参考 RT-Thread 实现）。

## A.3 Zephyr RTOS 的 device 模型

Zephyr 是 Linux 基金会托管的开源 RTOS，专门跑在资源受限的 MCU 上（几十 KB RAM 起步）。它要解决的问题是 Linux 的微缩版：怎么让一个 RTOS 同时支持 Nordic nRF52、TI MSP430、Espressif ESP32 等几百种 MCU 的 GPIO / UART / SPI / I²C / 定时器外设。

Zephyr 的解法叫 **device model**。核心抽象在 `include/zephyr/device.h`：

```c
struct device {
	const char *name;
	const void *config;
	const void *api;       /* 指向 driver API 结构体（ops 表）*/
	void *data;
};
```

`api` 字段就是 ops 表，叫 "API"。注意四个字段的设计：`name` 是字符串名，`config`（编译期常量配置）和 `data`（运行期可变数据）分开存储，这是嵌入式特有的优化。`config` 通常放 ROM（Flash），`data` 通常放 RAM。教学版常把两者混在一起，工业 RTOS 把它们物理分离能节省紧张的 RAM。

GPIO 子系统的 driver API：

```c
struct gpio_driver_api {
	int  (*pin_configure)(const struct device *port,
	                       gpio_pin_t pin, gpio_flags_t flags);
	int  (*port_get_raw)(const struct device *port,
	                     gpio_port_value_t *value);
	int  (*port_set_masked_raw)(const struct device *port,
	                             gpio_port_pins_t mask,
	                             gpio_port_value_t value);
	int  (*port_set_bits_raw)(const struct device *port,
	                          gpio_port_pins_t pins);
	int  (*port_clear_bits_raw)(const struct device *port,
	                             gpio_port_pins_t pins);
	int  (*port_toggle_bits)(const struct device *port,
	                          gpio_port_pins_t pins);
	int  (*pin_interrupt_configure)(const struct device *port,
	                                 gpio_pin_t pin,
	                                 enum gpio_int_mode mode,
	                                 enum gpio_int_trig trig);
	int  (*manage_callback)(const struct device *port,
	                         struct gpio_callback *cb, bool set);
	uint32_t (*get_pending_int)(const struct device *dev);
};
```

11 个虚方法。比 Linux 内核 GPIO 抽象简单（嵌入式不需要那么多花样），但比这本书 ch01 教学版复杂。

应用层调用 `gpio_pin_set(dev, pin, 1)` 展开后：

```c
const struct gpio_driver_api *api = dev->api;
api->port_set_bits_raw(dev, BIT(pin));
```

跟 Linux 的 `gc->set(gc, offset, value)` 同构，跟工业项目的 `me->ops->velocity_set(me, 200)` 同构。

### A.3.1 Zephyr 的 DEVICE_DT_INST_DEFINE 宏

Zephyr 把 initcall 风格做得更工业化。一个 GPIO 驱动注册自己的方式：

```c
DEVICE_DT_INST_DEFINE(idx,
                      gpio_stm32_init,           /* init 函数 */
                      NULL,                       /* pm 回调 */
                      &gpio_stm32_data_##idx,    /* 实例数据 */
                      &gpio_stm32_cfg_##idx,     /* 实例配置 */
                      PRE_KERNEL_1,               /* 初始化等级 */
                      CONFIG_GPIO_INIT_PRIORITY,  /* 优先级 */
                      &gpio_stm32_driver_api);    /* ops 表 */
```

`DEVICE_DT_INST_DEFINE` 这个宏展开后会做几件事：

1. 在 `.z_init_PRE_KERNEL_1_xxx` 段塞一个函数指针，启动时自动调用
2. 在 `.device_states` 段塞一个 `struct device` 实例
3. 把 `&gpio_stm32_driver_api` 链到 `dev->api`

跟工业项目的 `INIT_DEVICE_EXPORT(my_setup)` 加全局 `extern led_base_t *green_led;` 是一回事。差别只在 Zephyr 用编译期模板（多个 `_##idx` 后缀）批量生成实例，更精致。

### A.3.2 Zephyr 设备树：编译期展开式的设备描述

Zephyr 的"哪些设备存在 + 各自参数"信息来自 `.dts` 设备树文件，但和 Linux 不同的是：Linux 在运行时解析 dtb 二进制，Zephyr 在编译期通过宏展开把 dts 的内容变成 C 字面量。

某块板的 dts 写：

```
&i2c1 {
    status = "okay";
    clock-frequency = <I2C_BITRATE_FAST>;

    temp_sensor: max31827@5a {
        compatible = "maxim,max31827";
        reg = <0x5a>;
    };
};
```

Zephyr 的工具链（`gen_defines.py`）把这一段在编译期翻译成 C 宏：

```c
#define DT_NODELABEL_temp_sensor_REG_0_VAL_ADDRESS 0x5a
#define DT_NODELABEL_temp_sensor_BUS DT_NODELABEL_i2c1
```

驱动代码用 `DT_INST_REG_ADDR(idx)` 这种宏在编译期就把 0x5a 嵌进函数体，运行时没有任何解析开销。这是 Zephyr 比 Linux 多走的一步：把"设备描述 + 设备实例化"完全推到编译期，运行时只剩下 ops 表派发。

这跟工业项目的 `*_cfg.c` 是不同实现方式但同一思想：把"系统里有什么设备"从代码里抽出来集中描述，应用层和驱动层不依赖具体描述。

## A.4 GObject：把 C++ class 体系完整模拟出来

GObject 是 GTK / GNOME 桌面环境的对象系统，用纯 C 实现。它走得最远：不只支持单继承 + ops 表，还支持运行时类型识别、属性系统、信号系统，把 C++ class 体系完整模拟出来。

GObject 的核心数据结构（简化示意，真实代码在 `glib-2.0/gobject/gtype.h`）：

```c
typedef struct _GObject GObject;
typedef struct _GObjectClass GObjectClass;

struct _GObject {
	GTypeInstance  g_type_instance;     /* 实例数据 */
	volatile guint ref_count;
	GData         *qdata;
};

struct _GObjectClass {
	GTypeClass    g_type_class;          /* 类元信息 */

	/* 虚方法 */
	GObject *(*constructor)(GType type,
	                        guint n_construct_properties,
	                        GObjectConstructParam *construct_properties);
	void    (*set_property)(GObject *object, guint property_id,
	                        const GValue *value, GParamSpec *pspec);
	void    (*get_property)(GObject *object, guint property_id,
	                        GValue *value, GParamSpec *pspec);
	void    (*dispose)(GObject *object);
	void    (*finalize)(GObject *object);
	/* ... */
};
```

`GObject` 是所有 GTK 控件的根基类。`GObjectClass` 是它的 ops 表（GObject 的术语叫"class struct"）。

具体一个控件（比如 `GtkButton`）的继承链：

```
GObject
 └── GInitiallyUnowned
      └── GtkWidget
           └── GtkContainer
                └── GtkBin
                     └── GtkButton
```

每一层都按这本书 ch12 的写法把父类塞进自己的第一个字段：

```c
struct _GtkButton {
	GtkBin     parent_instance;   /* 父类实例 */
	/* GtkButton 自己的字段 */
	gchar     *label;
	guint      relief : 2;
	guint      use_underline : 1;
	/* ... */
};

struct _GtkButtonClass {
	GtkBinClass parent_class;     /* 父类的 ops 表 */
	/* GtkButton 自己的虚方法 */
	void (*pressed)  (GtkButton *button);
	void (*released) (GtkButton *button);
	void (*clicked)  (GtkButton *button);
	void (*activate) (GtkButton *button);
};
```

向上转型：

```c
GtkButton *button = gtk_button_new_with_label("OK");
GtkWidget *widget = (GtkWidget *)button;     /* 子类直接转父类 */
```

向下转型：

```c
GtkWidget *widget = ...;
GtkButton *button = G_TYPE_CHECK_INSTANCE_CAST(widget,
                                                GTK_TYPE_BUTTON,
                                                GtkButton);
```

`G_TYPE_CHECK_INSTANCE_CAST` 这个宏背后是运行时类型检查（RTTI），底下用了一份全局的 type ID 注册表。它比单纯的 C 强转多了一道安全检查："你确认这个 widget 真的是 GtkButton 吗？"

这是 GObject 比裸 C OOP 多的最重要一层：**运行时类型系统**。每个 GObject 类型在系统里注册一个 `GType`（实质是个 uintptr_t），父子关系存在一份全局表里。`G_TYPE_CHECK_INSTANCE_CAST` 在运行时按这份表检查"`widget` 的 GType 是不是 GtkButton 或它的子类"，是就成立，否则给 warning 并返回 `NULL`。

工业项目和 Linux 内核一般不做这一层，因为 RTTI 有运行时开销（几十纳秒每次转型），嵌入式资源紧张不愿付。GObject 服务的桌面 GUI 场景对类型安全的需求高过性能，所以付得起。

### A.4.1 GObject 的虚函数调用

调一个虚方法：

```c
gtk_button_clicked(button);
```

展开后大致是：

```c
void gtk_button_clicked(GtkButton *button)
{
	GtkButtonClass *klass = GTK_BUTTON_GET_CLASS(button);
	klass->clicked(button);
}
```

`GTK_BUTTON_GET_CLASS(button)` 这个宏沿着 `button->parent_instance.g_type_instance.g_class` 的指针链拿到 ops 表（在 GObject 术语里叫 class struct）。

完全是 ch11 的多态：vptr 拿出 vtable，vtable 里取函数指针，间接跳转。

GObject 比这本书的 `me->ops->fn(me)` 多套了几层（type system + class system + property system），但抽象骨架完全一致。

### A.4.2 GObject 的信号系统

GObject 还有一层这本书没讲过的能力：**信号（Signal）**，跨对象的回调机制。注册：

```c
g_signal_connect(button, "clicked",
                 G_CALLBACK(on_button_clicked), user_data);
```

按下按钮时，GObject 内部 emit `"clicked"` 信号，所有订阅者自动收到。

这跟工业项目的 `fan_status_change_cb_register` 是同一个东西的升级版：信号是"每个对象可以有任意多种事件，每种事件可以有任意多个订阅者"，回调注册是"每个对象固定一个事件、一个订阅者"。GObject 把这一层模式化了，写一行 `g_signal_connect` 就完成所有的注册 + 调度 + 解绑。

这是 OOP 模式从单一回调进化到事件总线的演化路径。工业项目 + Linux 内核里相同的模式叫 notifier chain（`include/linux/notifier.h`），骨架一致，命名不同。

## A.5 三个项目的对照表

四套代码（这本书 + Linux 内核 + Zephyr + GObject）放在一起：

| 概念 | 书 ch10-ch11 | Linux 内核 | Zephyr | GObject |
|---|---|---|---|---|
| 基类实例 | `struct led_base` | `struct file` / `struct gpio_chip` | `struct device` | `GObject` |
| ops 表 | `struct led_ops` | `struct file_operations` / `struct gpio_chip` 内嵌 | `struct gpio_driver_api` | `GObjectClass` 中的虚方法字段 |
| ops 字段 | `me->ops` | `file->f_op` / `gc->set` | `dev->api` | `instance->g_class` |
| 调用 | `me->ops->on(me)` | `f->f_op->read(f, ...)` | `api->port_set_bits_raw(dev, ...)` | `klass->clicked(button)` |
| 子类嵌入基类 | `struct led { struct led_base base; ... }` | `struct cdev { struct kobject kobj; ... }` | （字段 + dt 配置组合） | `struct GtkButton { GtkBin parent_instance; ... }` |
| 向下转型 | `(struct led *)me` 或 `container_of` | `container_of(ptr, struct cdev, kobj)` | （通过 `dev->data`） | `G_TYPE_CHECK_INSTANCE_CAST` |
| initcall 等级 | INIT_DEVICE_EXPORT 等 7 级 | core/postcore/.../late 7 级 | PRE_KERNEL_1 / POST_KERNEL / APPLICATION | 编译期 + GType 注册时执行 |
| 注册机制 | `extern led_base_t *green_led;` | `register_chrdev` + `module_init` / `gpiochip_add_data` | `DEVICE_DT_INST_DEFINE` 宏 | `g_type_register_static` |
| 类型检查 | 无（裸强转） | 无（裸强转） | 无（裸强转） | RTTI（`G_TYPE_CHECK_INSTANCE_CAST`） |
| 事件机制 | 单 cb 字段 | callback / notifier_chain | callback 数组 | signal / property |

10 行对照下来，**结构完全同构**。Linux 内核、Zephyr、GObject 的 C 代码风格各有差异（缩进、命名、宏怎么写），但骨架都是 struct + me + ops 表 + 子类嵌入基类 + container_of 转型 + 链接器自动初始化。

每个项目又在标准骨架上按自己的语境加了一些扩展：Linux 加 driver model + 内核态 / 用户态隔离，Zephyr 加编译期 dts 展开，GObject 加 RTTI + signal 总线。这些扩展不是"另一套架构"，是同一套架构的细化。

## A.6 这套模式的历史

这套写法不是任何单一团队发明的，是 1980 至 2000 年代 C 工程界几代人逐步沉淀出来的。

最早的形式可以追到 1980 年代 BSD UNIX 的设备驱动框架（`struct cdevsw`，是 `file_operations` 的祖先）。1990 年代 GTK / GNOME 项目把 OOP 模式系统化，催生 GObject。同期 Linux 内核 0.x 年代，Linus 把 BSD 的 `cdevsw` 引入 Linux 并演化成 `file_operations`。2000 年代各家 RTOS（FreeRTOS / RT-Thread / Zephyr 等）继承这套模式并按嵌入式资源限制做精简版。

到 2020 年代，这套模式已经是 C 大型项目的事实标准。任何想做"可扩展、可移植、可维护"的 C 项目，最后都会收敛到这种形态。区别只在每个项目按自己的语境做了微调（命名、宏的写法、是否有 RTTI 等）。

中国市面上的嵌入式书绝大多数没有讲到这一层。它们停留在"配寄存器"或"调用 HAL 函数"，没有再往上抽象。所以工程师从书里学到的代码组织方式和真实工业项目之间有断层。

这本书前 18 章铺垫的所有抽象，加 ch19-ch20 的工业代码，加这一附录的三大开源对照，目的就是把这个断层补上：让你看完之后，看 Linux 内核源码不陌生，看 Zephyr 不陌生，看 GTK 不陌生，看任何一份工业 C 代码不陌生。

## A.7 怎么读这三个项目的源码

如果你想自己动手读这三个项目的源码：

**Linux 内核（推荐入口）：**

- 从 `include/linux/fs.h` 看 `struct file_operations`
- 从 `drivers/char/mem.c`（最简单的字符设备驱动 `/dev/null`）开始读，几百行
- 看明白后再读 `drivers/gpio/gpiolib.c`，看 `gpio_chip` 怎么注册和调用
- 最后看 `init/main.c` 里 `do_initcalls()`，看 7 级 initcall 怎么调度

**Zephyr：**

- 从 `include/zephyr/device.h` 看 `struct device` 抽象
- 看 `drivers/gpio/gpio_stm32.c`（如果用 STM32 平台），从 `DEVICE_DT_INST_DEFINE` 入手
- 一个 driver 可以一晚上读完

**GObject：**

- 从 `glib-2.0/gobject/gobject.h` 看 `struct _GObject`
- 从 `glib-2.0/gobject/gtype.h` 看 type system 注册流程
- 难度比 Linux / Zephyr 高，但只要前两个看明白，GObject 一周能搞清

读这三个项目的诀窍：**永远先找 ops 表（vtable）**。ops 表里的方法名告诉你这个对象支持哪些操作。剩下的 base struct 字段都是私有数据，用得到的时候再细看。

看到一个 callback 就找它在哪里被调用、谁注册了它（通常是另一个驱动或框架核心）。

看到一个 `container_of` 就停下来想：这个函数被某个 framework 调用，传进来的是基类指针，但函数里要用子类字段，所以要把基类指针"翻"回子类指针。

养成这套阅读习惯，你已经超过 90% 的中国嵌入式工程师。

## 下一篇

[附录 B · STM32 完整工程](B-STM32完整工程.md)
