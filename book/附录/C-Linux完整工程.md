# 附录 C · Linux 完整工程 · Raspberry Pi 4B

## C.0 关于这一份附录

附录 C 给读者一份在 Linux 6.6 主线上跑的完整工程。参考板是 Raspberry Pi 4B（这一块板子在主线 6.6 是 first-class，GPIO / I2C / SPI / PWM / LED 全走主线驱动，不需要 vendor fork）。配套代码在 `industrial-linux/` 下，包含一个自己写的内核 platform driver 模块 `leds-status.c`，加四个递进的 demo。本附录的目标不是从零讲 Linux 内核开发，而是给读者一份能直接跑、能按章对照的参考工程：读到 ch12 想验证父类首字段的读者，可以从 `leds-status.c` 的 `struct status_led_data` 起步；读到 ch17 想看 `module_init` 真实展开的读者，可以从 `module_platform_driver` 一行宏起步。

诚实边界声明：配套代码对照 Linux 6.6 上游写，参照主线已支持的 Raspberry Pi 4B，读者按附录步骤应当原样跑通，有出入欢迎到 GitHub 提 issue。

跟附录 B（裸机 Zephyr 必须自抽 platform）正好相反：**Linux 内核已经把 platform 抽象做完了**。应用层和驱动层都不需要再抽一层 `platform_pin_xxx()`，直接用内核现成的 `led_classdev` / `platform_driver` / sysfs / libgpiod。这一份附录就是把"内核做完别再抽"在真实主线代码上落实。读者把附录 B 的 STM32 工程跟附录 C 的 RPi 工程对照看一遍，能立刻看清楚同一组 OOP 抽象在两种环境下的归属差别：裸机环境每一层都得自己写一次，Linux 环境每一层都已经有内核的成品，自己写反而是重复劳动。

## C.1 5 分钟跑通

```bash
# 1. 烧 Raspberry Pi OS 64-bit (基于 6.6 内核)
#    用 RPi Imager 即可

# 2. 装 kernel headers + 编译工具 + libgpiod
sudo apt install raspberrypi-kernel-headers build-essential libgpiod-dev

# 3. clone 本书配套代码
git clone https://github.com/ZhaoChengBo/zhaoming-embedded.git ~/zhaoming-embedded

# 4. build leds-status.ko
cd ~/zhaoming-embedded/industrial-linux/ch20-leds-status
make

# 5. 编译 dts overlay·告诉内核 GPIO17 上有一个 status-led 兼容设备
sudo dtc -@ -O dtb -o leds-status.dtbo leds-status-overlay.dts
sudo dtoverlay leds-status.dtbo

# 6. insmod 模块
sudo insmod leds-status.ko

# 7. 验证
ls /sys/class/leds/                                          # 看见 status-led
echo 1 | sudo tee /sys/class/leds/status-led/brightness    # LED 亮
echo 0 | sudo tee /sys/class/leds/status-led/brightness    # LED 灭
```

期望硬件：GPIO17 引脚串 220Ω 限流电阻，再串一颗 LED 到 GND。`echo 1` 时 LED 亮，`echo 0` 时熄灭。

GPIO17 是教学 demo 用的引脚，不是 RPi 板载 ACT 灯（板载 ACT 在 GPIO42，跑 heartbeat trigger，别动）。

## C.2 工程目录结构

```
industrial-linux/
├── README.md
├── ch20-leds-status/                # Demo 1·写自己的内核驱动
│   ├── Makefile
│   ├── leds-status.c
│   ├── leds-status-overlay.dts
│   └── README.md
├── ch20-demo2-libgpiod/               # Demo 2·sysfs vs libgpiod 同硬件两接口
│   ├── Makefile
│   └── blink_libgpiod.c
├── ch20-demo3-gdb/                    # Demo 3·container_of 现场抓
│   ├── README.md
│   └── debug.gdb
└── ch20-demo4-initcall/               # Demo 4·module_init 链路追踪
    └── trace_initcall.sh
```

每个子目录的功能：

- **ch20-leds-status**：自己写一个 platform driver，注册一个新的 `/sys/class/leds/status-led/`，让 ch12 / ch13 / ch15 / ch17 全套 OOP 抽象在内核里跑通
- **ch20-demo2-libgpiod**：同一颗 LED，先用内核驱动 + sysfs 控制，再 rmmod 后用用户态 libgpiod 直接拍 GPIO17。两种接口完全互斥，看清楚"内核占了 GPIO 用户态就拿不到"
- **ch20-demo3-gdb**：QEMU virt + gdb-multiarch 现场断点，打印 `&led->cdev == led_cdev` 验证 ch13 container_of 字节级一致
- **ch20-demo4-initcall**：用 dmesg / kallsyms / ftrace 三件套追 `module_platform_driver` 宏展开后的 initcall 链路，对应 ch17 教学版 initcall 表的内核成品

## C.3 关键文件解读

### C.3.1 ch20-leds-status/Makefile

外部模块标准 8 行：

```makefile
obj-m += leds-status.o
KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
```

逐行解释：

- `obj-m += leds-status.o` 是 Linux kbuild 风格，告诉 kbuild 把 `leds-status.c` 编成 `leds-status.ko`
- `KDIR := /lib/modules/$(shell uname -r)/build` 定位当前内核的 headers + 构建脚本
- `$(MAKE) -C $(KDIR) M=$(PWD) modules` 是外部模块的标准模式：去 KDIR 跑 modules target，但模块源码在当前目录（M=$(PWD)）

### C.3.2 ch20-leds-status/leds-status.c

整个文件大约 50 行，全文如下：

```c
// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>

struct status_led_data {
    struct led_classdev cdev;       /* 父类首字段·向上转型零代价 */
    struct gpio_desc   *gpiod;
};

static void status_led_brightness_set(struct led_classdev *led_cdev,
                                    enum led_brightness value)
{
    struct status_led_data *led =
        container_of(led_cdev, struct status_led_data, cdev);
    gpiod_set_value(led->gpiod, value ? 1 : 0);
}

static int status_led_probe(struct platform_device *pdev)
{
    struct status_led_data *led;

    led = devm_kzalloc(&pdev->dev, sizeof(*led), GFP_KERNEL);
    if (!led)
        return -ENOMEM;

    led->gpiod = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(led->gpiod))
        return PTR_ERR(led->gpiod);

    led->cdev.name           = "status-led";
    led->cdev.max_brightness = 1;
    led->cdev.brightness_set = status_led_brightness_set;

    return devm_led_classdev_register(&pdev->dev, &led->cdev);
}

static const struct of_device_id status_led_of_match[] = {
    { .compatible = "status-led", },
    { },
};
MODULE_DEVICE_TABLE(of, status_led_of_match);

static struct platform_driver status_led_driver = {
    .probe  = status_led_probe,
    .driver = {
        .name           = "leds-status",
        .of_match_table = status_led_of_match,
    },
};
module_platform_driver(status_led_driver);
MODULE_LICENSE("GPL");
```

四段对照本书前 18 章的 OOP 抽象：

**1. 父类作首字段**

```c
struct status_led_data {
    struct led_classdev cdev;       /* 父类作为第一字段 */
    struct gpio_desc   *gpiod;
};
```

`struct led_classdev` 在第一字段，`offsetof(struct status_led_data, cdev) == 0`。任何 `struct status_led_data *` 直接转 `struct led_classdev *` 拿到的地址相等，向上转型零代价。这是 Linux 6.6 `drivers/leds/leds-gpio.c` 里 `struct gpio_led_data` 的同款布局，全树几十种 LED 子类（leds-gpio / leds-pwm / leds-bcm6328 / leds-mt6323 等）都是这个写法。

**2. container_of 反推**

```c
static void status_led_brightness_set(struct led_classdev *led_cdev,
                                    enum led_brightness value)
{
    struct status_led_data *led =
        container_of(led_cdev, struct status_led_data, cdev);
    gpiod_set_value(led->gpiod, value ? 1 : 0);
}
```

内核回调拿到的是父类指针 `struct led_classdev *led_cdev`，`container_of` 把它反推成子类指针 `struct status_led_data *led`，然后操作子类私有的 `gpiod`。`container_of` 的内核定义在 `include/linux/container_of.h`：底层就是 `((type *)(__mptr - offsetof(type, member)))`，跟本书 ch13 字节级一致。父类在第一字段时 `offsetof` 为 0，`container_of` 数学上等于强转，但写 `container_of` 是为了将来父类位置变了不用改回调。

**3. probe 注册子类到父类**

```c
led->cdev.brightness_set = status_led_brightness_set;
return devm_led_classdev_register(&pdev->dev, &led->cdev);
```

把子类自己的 `status_led_brightness_set` 函数指针挂到父类的 `brightness_set` 字段上，再把 `&led->cdev`（父类视图）注册进 `led_classdev` 子系统。从此 `/sys/class/leds/status-led/brightness` 任何一次写入都会通过父类 dispatch 回到子类的实现。这就是 ch15 接口契约的内核成品。

**4. module_platform_driver 一行宏**

```c
module_platform_driver(status_led_driver);
```

这一行展开三层（`include/linux/platform_device.h` → `include/linux/device/driver.h` → `include/linux/module.h`），最终等价于：

```c
static int __init status_led_driver_init(void) {
    return platform_driver_register(&status_led_driver);
}
module_init(status_led_driver_init);
/* module_init 进一步展开为 __initcall */
```

也就是：`module_platform_driver(status_led_driver)` ≡ `__initcall(status_led_driver_init)`。这正是本书 ch17 教学版 initcall 表的内核成品，`module_init` 本身就是 `__initcall` 的别名。

### C.3.3 leds-status-overlay.dts

```dts
/dts-v1/;
/plugin/;

/ {
    compatible = "brcm,bcm2711";

    fragment@0 {
        target-path = "/";

        __overlay__ {
            status_led: status-led {
                compatible = "status-led";
                gpios = <&gpio 17 0>;       /* GPIO17·active high */
                status = "okay";
            };
        };
    };
};
```

要点：

- `compatible = "status-led"` 是 of_match 的关键字，跟驱动里 `status_led_of_match[]` 那一行字面对齐，匹配上 probe 才会被调用
- `gpios = <&gpio 17 0>` 指定 GPIO17，flags 0 是 active high
- 编译命令 `dtc -@ -O dtb -o leds-status.dtbo leds-status-overlay.dts`，`-@` 让符号引用（`&gpio`）保留在产物里，`dtoverlay` 才能后期解析
- 加载用 `sudo dtoverlay leds-status.dtbo`（运行时挂载）；要开机自动挂载，把 dtbo 拷到 `/boot/overlays/`，再在 `/boot/config.txt` 里加一行 `dtoverlay=leds-status`

## C.4 4 个 demo 的 build / 跑命令 / 期望输出

| Demo | build / 跑命令 | 期望输出 |
|---|---|---|
| 1 · leds-status.ko | `cd ch20-leds-status && make && sudo dtoverlay leds-status.dtbo && sudo insmod leds-status.ko` | `/sys/class/leds/status-led/` 出现，`echo 1 \| sudo tee .../brightness` 点亮 LED |
| 2 · libgpiod | `cd ch20-demo2-libgpiod && make && sudo ./blink_libgpiod` | rmmod 后 LED 闪 20 下；insmod 内核驱动后 libgpiod 报 `Device or resource busy`（GPIO17 已被内核占用） |
| 3 · gdb container_of | QEMU virt 启 vmlinux + `gdb-multiarch vmlinux` + `target remote :1234` + `b status_led_brightness_set` | gdb 打印 `&led->cdev == led_cdev`（cdev 在第一字段，offset 0，两地址相同） |
| 4 · initcall | `bash ch20-demo4-initcall/trace_initcall.sh` | dmesg 看到 `initcall ... returned 0`；ftrace 看到 `status_led_probe` 被 `platform_drv_probe` 调用 |

每个 demo 的具体讲解见各自 `README.md`。Demo 2 的核心观察是"同一颗 LED 两种接口完全互斥"：先 rmmod 再 libgpiod 直接拍 GPIO17 → LED 闪；再 insmod 后 libgpiod 拒绝（GPIO 已被内核占用）→ 只能走 sysfs。这一组对照让读者切身体会内核驱动和用户态直接控制 GPIO 的关系，对应 ch16 § 16.14（应用层 vs 内核层判断三步）。一颗 GPIO LED 这种通用外设，工业上 99% 的场景在用户态调 libgpiod 就够，只有把驱动写成内核态才能享受 led trigger / sysfs 自动节点这些子系统配套能力，所以"是否值得写内核驱动"是个工程判断题，不是技术能力题。Demo 3 把 ch13 字节级一致从教学代码搬到内核现场，gdb 打印的两个地址完全相等就是父类在第一字段时 `offsetof` 为 0 的最直观证据。Demo 4 把 ch17 initcall 链路从教学版宏搬到 `module_platform_driver` 宏，读者顺着 dmesg / kallsyms / ftrace 三条线索看完，会发现内核启动时数千个 initcall 不是魔法，就是一张按 level 排序的函数指针表加一个 for 循环。

## C.5 没有 RPi 怎么办

诚实给出三套方案：

- **WSL2 / Linux PC**：可以 build 模块（装 kernel headers 即可），但不能 insmod 跑（除非自编内核 + 自加 dts overlay）。Demo 4 的 `trace_initcall.sh` 在任何 Linux 主机上都能跑，因为它读 `/proc/kallsyms` 和 `/sys/kernel/debug/tracing/`，不依赖 RPi 硬件
- **QEMU virt + 主线 Linux**：跑 Demo 3 gdb 调试最理想，不需要 RPi 真板。`qemu-system-aarch64 -M virt -cpu cortex-a72 -kernel arch/arm64/boot/Image -append "console=ttyAMA0 nokaslr" -nographic -s -S` 启动后接 `gdb-multiarch vmlinux`，`target remote :1234` 即可
- **完全不跑·只读源**：附录 C 每段代码都从 Linux 6.6 主线 / RPi 主线 dts 真源里抓出来。读者顺着下面的 permalink 一字不差核对：
  - `https://github.com/torvalds/linux/blob/v6.6/include/linux/leds.h`
  - `https://github.com/torvalds/linux/blob/v6.6/drivers/leds/led-class.c`
  - `https://github.com/torvalds/linux/blob/v6.6/drivers/leds/led-core.c`
  - `https://github.com/torvalds/linux/blob/v6.6/drivers/leds/leds-gpio.c`
  - `https://github.com/torvalds/linux/blob/v6.6/include/linux/container_of.h`
  - `https://github.com/torvalds/linux/blob/v6.6/include/linux/platform_device.h`
  - `https://github.com/torvalds/linux/blob/v6.6/include/linux/device/driver.h`
  - `https://github.com/torvalds/linux/blob/v6.6/include/linux/module.h`
  - `https://github.com/torvalds/linux/blob/v6.6/arch/arm/boot/dts/broadcom/bcm2711-rpi-4-b.dts`

## C.6 常见坑

1. **kernel headers 缺失**：`sudo apt install raspberrypi-kernel-headers`。`uname -r` 拿到的版本要跟 headers 包匹配，跨大版本升级后要重装 headers
2. **dts overlay 没加载**：开机自动挂载要在 `/boot/config.txt` 加 `dtoverlay=leds-status` 并把 `.dtbo` 拷到 `/boot/overlays/`；运行时挂载用 `sudo dtoverlay leds-status.dtbo` 或 `sudo dtoverlay leds-status`
3. **module signing**：开了 `CONFIG_MODULE_SIG_FORCE` 才需要签名，RPi OS 默认没强制。如果 `insmod` 报 `Required key not available`，关掉 secure boot 或自签
4. **GPIO17 被占用**：先 `pinctrl get 17` 确认没被别的 driver 占（旧 dtb 里如果把 17 给了别的功能会冲突）。同样的命令也能确认 dts overlay 加载后 GPIO17 是不是切到了 `output` 模式
5. **`/sys/class/leds/...` 不出现**：dmesg 看 probe 是否被调用（of_match 是否匹配）。常见错位是 dts 里写 `compatible = "status-led"` 漏了逗号，或者驱动的 `status_led_of_match[].compatible` 字符串拼错
6. **brightness_set 没生效**：检查 `cdev.max_brightness` 不是 0。`led_classdev_register` 默认会把 0 改写成 `LED_FULL=255`，但有些代码路径会先读这一字段做 clamp，写了再说稳

## C.7 RPi 4B 主线驱动状态

| 外设 | 驱动文件 | 主线 6.6 状态 |
|---|---|---|
| GPIO | drivers/pinctrl/bcm/pinctrl-bcm2835.c | first-class |
| I2C | drivers/i2c/busses/i2c-bcm2835.c | first-class |
| SPI | drivers/spi/spi-bcm2835.c | first-class |
| PWM | drivers/pwm/pwm-bcm2835.c | first-class |
| LED | drivers/leds/leds-gpio.c + dts | first-class |

结论：RPi 4B 在主线 6.6 是 first-class，所有教学需要的外设走主线驱动，不需要 vendor fork。这也是这本书选 RPi 4B 做参考板的理由：教学版代码读上游、读者跑同款上游，认知完全闭环。

## C.8 这一份附录在全书的位置

附录 C 跟附录 B 配对，是这本书工程判断力教学的两个工程兑现：

- **附录 B（裸机 STM32）**：Zephyr 的 device subsystem 在某些受限场景仍要自抽 platform，那一份工程演示"必须自抽"长什么样
- **附录 C（Linux 6.6 + RPi 4B）**：Linux 内核已经把 platform 抽象做完了，应用层和驱动层都不再自抽，那一份工程演示"内核做完别再抽"长什么样

读完两份附录，读者能切身体会本书 ch15 § 15.16 / ch16 § 16.13 / ch16 § 16.14 的核心 takeaway：抽不抽 platform 层不是教条，要看你跑在哪个环境。判断依据是"内核有没有把 platform 抽象做完"。看到一份 Linux 应用层代码自抽 `platform_pin_xxx()`，第一反应是"这一层是不是多余"；看到一份裸机 STM32 代码不抽 platform 层、应用直接调 `HAL_GPIO_WritePin`，第一反应是"换芯片就崩"。这两种反应分得清，工程判断力就立住了。这也是本书前 18 章一直在铺的一条线：OOP 不是写法清单，是判断力训练。判断"谁该抽这一层"比写出"这一层怎么抽"重要十倍。
