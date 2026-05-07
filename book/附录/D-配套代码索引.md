# 附录 D · 配套代码索引 + 视频清单 + 编译运行指南

这一附录是全书的"导航地图"。你在任何一章想找配套代码、想找对应视频、想跑某一章的代码，全在这一附录。

## D.1 配套代码包索引

每一章对应一个独立目录在 `oop-in-c/code/<ch>-<topic>/` 下。结构都按 ch01 模板：

```
oop-in-c/code/<ch>-<topic>/
├── pc/                 完整可跑的 PC 模拟版
│   ├── Makefile
│   ├── README.md
│   ├── led.h / led.c
│   ├── main.c
│   └── （根据章节，可能含 motor.h / motor.c 等）
├── stm32-snippet/      STM32 HAL 等效片段（不是完整工程）
│   └── led_stm32.c
├── linux-snippet/      Linux 用户态等效片段
│   └── led_linux.c
└── README.md
```

完整列表：

| 章 | 目录 | 一句话内容 |
|---|---|---|
| ch01 | `01-three-leds/` | struct + me 指针的最朴素形态，三颗 LED 共用一份代码 |
| ch02 | `02-static-hiding/` | static 关键字 + 信息隐藏，把字段从 .h 移到 .c |
| ch03 | `03-handwritten-class/` | led_init / led_deinit 命名固化，引入 motor 模块演示同套路 |
| ch04 | `04-data-classification/` | 数据三级归位（实例字段 / 模块 static / 只读 const），按场景选静态 / 池 / 动态分配 |
| ch05 | `05-hal-mapping/` | 不引入新概念，打开 ST HAL 源码漫游 BSRR 寄存器 |
| ch06 | `06-inherit-pain/` | 继承痛点，引入 `struct led_base` 提取共有字段 pin |
| ch07 | `07-function-pointer/` | 函数指针入门，独立变量 `void (*fp)(int)` 演示存号码 + 拨号 |
| ch08 | `08-callback/` | 函数指针传参，注册 / 拆解 callback |
| ch09 | `09-ops-table/` | ops 操作表雏形，`struct led_ops { on, off, toggle }` |
| ch10 | `10-vptr/` | ops 放进对象，`const struct led_ops *ops` 字段 |
| ch11 | `11-polymorphism/` | 多态完整图景，`led->ops->on(led)` runtime dispatch |
| ch12 | `12-upcasting/` | 向上转型，`(struct led_base *)led` 子类指针转基类 |
| ch13 | `13-container-of/` | container_of 宏，从基类指针拿回子类 |
| ch14 | `14-pure-virtual/` | 纯虚 / 抽象类，`int (*on)(...);` 没赋值 |
| ch15 | `15-platform/` | platform 抽象到底，GPIO / PWM / I2C 三种硬件混搭，应用层 0 修改换硬件方案 |
| ch16 | `16-linux-style/` | Linux 内核风格，引用 `struct gpio_chip` |
| ch17 | `17-initcall/` | 链接自动初始化，模仿 Linux 内核的 module_init 机制 |
| ch18 | `18-roadmap/` | 全书地图回顾，一颗 LED 18 章演化路径全景图 |

每个 `pc/README.md` 描述这一章演示什么、跑出来你应该看到什么。每个 `pc/Makefile` 一行 `make` 即可编译。

## D.2 工业代码包索引

工业代码在 `industrial/` 下：

| 目录 | 对应章节 | 内容 |
|---|---|---|
| `industrial/led_basic/` | ch19 19.1 | LED 最小继承范例，`led_base + led_gpio` |
| `industrial/motor_24vfuncs/` | ch20 20.1 - 20.4 | Motor 驱动 24 虚方法 + 工作线程 + 三种回调 |
| `industrial/platform_layer/` | ch20 20.6 | UART / I²C / SPI 统一抽象 + initcall 7 级 |
| `industrial/stm32_full/` | 附录 B | STM32F407 完整工程，全书所有抽象一次跑通 |
| `industrial/linux_full/` | 附录 C | Linux 用户态完整工程，全书所有抽象在 Linux 跑通 |

`industrial/README.md` 是工业代码总入口，描述"工业代码和教学版的 9 个差距"。

## D.3 视频清单

每章对应一期视频，发在 B 站 / 抖音 / 视频号「兆鸣嵌入式」频道。

| 章节 | 视频标题（B 站 / 抖音 / 视频号搜「兆鸣嵌入式」） | 时长 |
|---|---|---|
| ch01 | C 语言·三个 LED 你写了三份代码 | ~3 分钟 |
| ch02 | C 语言·你同事改了一行代码 LED 全乱了 | ~3 分钟 |
| ch03 | C 语言·你用 C 手搓了一个 class | ~3 分钟 |
| ch04 | C 语言·你的全局变量该死了 | ~3 分钟 |
| ch05 | C 语言·HAL 库几千个函数就一个套路 | ~3 分钟 |
| ch06 | C 语言·你的代码一半是重复的 | ~3 分钟 |
| ch07 | C 语言·写死的函数怎么换 | ~3 分钟 |
| ch08 | C 语言·函数指针传参 | ~3 分钟 |
| ch09 | C 语言·ops 操作表 | ~3 分钟 |
| ch10 | C 语言·ops 放进对象 | ~3 分钟 |
| ch11 | C 语言·多态 | ~3 分钟 |
| ch12 | C 语言·向上转型 | ~3 分钟 |
| ch13 | C 语言·向下转型·container_of | ~3 分钟 |
| ch14 | C 语言·虚函数不实现 | ~3 分钟 |
| ch15 | C 语言·换硬件不改应用 | ~3 分钟 |
| ch16 | C 语言·Linux 不难 | ~3 分钟 |
| ch17 | C 语言·链接自动初始化 | ~3 分钟 |
| ch18（全书地图回顾） | OOP 主体 18 章演化路径终篇 + SP 合集 18 期 | ~5+10 分钟 |

视频和书互相补强。视频更直观看口播节奏和类比的现场感。书里补了视频没讲透的细节、STM32 / Linux / 工业代码对照、内存布局、汇编层面分析。

## D.4 编译运行指南

### D.4.1 Windows 用户

装 GCC 工具链。两个常见方案选一个：

**方案 A: MinGW-w64**

搜索引擎搜"MinGW-w64 官网"，下载安装包，一路 next，勾"添加到 PATH"。装完后命令行 `gcc --version` 看到版本号即装好。

**方案 B: MSYS2**

搜索引擎搜"MSYS2 官网"，下载安装包。装完后开 MSYS2 MinGW 64-bit shell：

```bash
pacman -S mingw-w64-x86_64-gcc make
```

之后无论 cmd / PowerShell / Git Bash 都能用 `gcc`、`make`。

**方案 C: WSL2（Windows 10/11）**

在 PowerShell 里 `wsl --install` 装 Ubuntu，进 Ubuntu 后 `sudo apt install build-essential`。整个开发体验和 Linux 一样。

### D.4.2 Linux 用户

```bash
# Debian / Ubuntu
sudo apt install build-essential

# CentOS / RHEL
sudo yum groupinstall "Development Tools"
```

`gcc --version` 和 `make --version` 看到版本号即装好。

### D.4.3 跑任一章配套代码

三步法（每章一样）：

```bash
cd oop-in-c/code/01-three-leds/pc      # 换成你想跑的章
make
./demo
```

Windows 上跑出来的可执行文件叫 `demo.exe`，Linux 上叫 `demo`。

终端会滚出几十行 `[GPIO]` 和 `[LED]` 输出，跟章节正文末尾"跑一遍"那一节的预期输出一致。

### D.4.4 跑出错时的常见问题

| 报错 | 原因 | 解决 |
|---|---|---|
| `gcc: command not found` | GCC 没装或不在 PATH | 按 D.4.1 / D.4.2 重新装一遍 |
| `make: command not found` | Windows 上 MinGW 装的是 `mingw32-make` | 改名或建别名，或 MSYS2 装 make 包 |
| `fatal error: stdio.h: No such file` | GCC 装得不全 | 重装并选完整开发套件 |
| 中文路径报错 | 仓库克隆在含中文的路径 | 改克隆到纯英文路径，如 `D:\code\zhaoming` |
| 编译过了但运行报错 | Windows 上需要 `./demo.exe` | 加 `./` 前缀或用全名 |

还是不行，到 [GitHub Issues](https://github.com/ZhaoChengBo/zhaoming-embedded/issues) 或 [Gitee Issues](https://gitee.com/zhao-chengbo/zhaoming_embedded/issues) 提一个，附系统、GCC 版本、完整报错日志，我会回。

### D.4.5 跑 STM32 工程（附录 B）

需要：

- arm-none-eabi-gcc 工具链（搜索引擎搜"ARM GNU Toolchain"到 ARM 官网下载）
- STM32F407 Discovery 板（淘宝或正点原子等渠道）
- ST-Link 调试器（板上自带 V2-A）

编译：

```bash
cd industrial/stm32_full
make
```

烧录：

```bash
st-flash write build/firmware.bin 0x08000000
```

或用 STM32CubeProgrammer GUI。

### D.4.6 跑 Linux SBC 工程（附录 C）

在树莓派 / 香橙派等 SBC 上：

```bash
sudo apt install libgpiod-dev gpiod build-essential
cd industrial/linux_full
make
sudo ./build/demo
```

板上接 4 颗 LED + 220Ω 限流电阻到 GPIO（具体引脚见附录 C C.4 led_cfg.c）。

## D.5 看哪一章解决哪种问题（速查）

如果你不是从头读，而是有具体问题来书里查：

| 你的问题 | 看哪一章 |
|---|---|
| 几颗 LED 写了几份重复代码 | ch01 |
| 想保护 struct 字段不被外面乱改 | ch02 |
| 想把 C 写得像 class 一样 | ch03 |
| 全局变量太多想清理 | ch04 |
| 想看懂 STM32 HAL 库源码 | ch05 |
| 几个驱动有大量重复代码想抽共性 | ch06 |
| 想用函数指针实现简单回调 | ch07 |
| 想注册多个回调 / 让别人替我做某件事 | ch08 |
| 一个驱动有 5+ 个操作想统一管理 | ch09 |
| 想让对象自带操作表 | ch10 |
| 同一个接口想要多种实现 | ch11 |
| 子类指针往哪转、为什么这样转 | ch12 |
| 收到基类指针想拿回子类 | ch13 |
| 想强制子类必须实现某些方法 | ch14 |
| 想做一份代码多平台跑（PC / STM32 / Linux） | ch15 |
| 想知道 Linux 内核驱动怎么写 | ch16 |
| 想做大型项目自动初始化（避免 main 一长串 init） | ch17 |
| 想看全书演化路径全景图 | ch18 |
| 想看真实工业项目代码长什么样 | ch19 / ch20 |
| 想看 Linux 内核 / Zephyr / GObject 怎么写 OOP | 附录 A |
| 想要完整可跑的 STM32 工程 | 附录 B |
| 想要完整可跑的 Linux 工程 | 附录 C |

## D.6 推荐阅读顺序

**有经验工程师（写过 5 年以上 C）：**

跳读路径：ch01（确认基础）→ ch11（多态完整）→ ch15（platform）→ ch17（initcall）→ ch19/ch20（工业代码）→ 附录 A。两个晚上看完。

**初级嵌入式工程师（写过 1-3 年 C）：**

按章顺序读 ch01-ch18，每章读完跑一遍配套代码。第 11 章是分水岭，读到这里能感觉脑子里 OOP 这件事开始通了。一周到两周看完前 18 章。

**大学生 / 刚学完 C 语法：**

按章顺序读，每章不要跳，配套代码每行都看。读到 ch07-ch11（函数指针那一段）会觉得有点难，硬扛过去就豁然开朗。预计两到三周看完前 18 章。

**只想速通某个面试题：**

查 D.5 表，找对应章。每章正文 + 7 节"视频里没讲透的几个细节"是干货密度最高的部分。

## D.7 反馈与勘误

发现错误 / 有改进建议 / 想贡献一章：

到 [GitHub Issues](https://github.com/ZhaoChengBo/zhaoming-embedded/issues) 或 [Gitee Issues](https://gitee.com/zhao-chengbo/zhaoming_embedded/issues) 提 Issue，附章节、你的理解、你认为的问题。我会回。

读完哪章你觉得讲透了，哪章还差点意思，欢迎写出来。这是迭代下一版的最好材料。

## D.8 这本书的全景

读到这里你已经是这本书的"完整读者"。回头看一下：

```
序曲       5 分钟看见你的第一个 OOP LED
           （费曼"被自己说服"原则的入口）

ch01-05    封装篇        struct + me + static + 数据归位 + HAL 漫游
ch06       继承痛点      抽公因子的痛和爽
ch07-11    多态篇        函数指针 → callback → ops 表 → vptr → 多态
ch12-13    转型篇        向上转型（自动）+ 向下转型（container_of）
ch14       纯虚 / 抽象类
ch15       platform 抽象到底（高潮章·换硬件不改应用）
ch16       Linux 风格（你已经在写 Linux 风格代码）
ch17       链接自动初始化（4000 万行一招写完）
ch18       全书地图回顾

ch19-20    工业实战      工业控制板主控 + 子控的真实代码

附录 A     三大开源对照（Linux / Zephyr / GObject）·证明这套模式是世界标准
附录 B     STM32 完整工程·全书所有抽象一次跑通
附录 C     Linux 完整工程·跟 B 做 diff 看抽象的威力
附录 D     索引·这一份
```

20 章 + 4 附录加起来不到 30 万字。但读完之后，**任何一份工业 C 代码摆你面前，5 分钟看出它的骨架**。这就是这本书的目标：不靠堆量，靠把一件事讲透。

完结的话留给我：

我做嵌入式 11 年，一直觉得这套写法应该有人系统讲清楚。视频讲了，但视频留不住细节。书留住了。

如果你读完这本书觉得 OOP 没那么神秘，下次接手陌生项目能 5 分钟看懂骨架；如果你读完去 GitHub 给某个开源项目交了一份干净的 driver；如果你下次面试讲 container_of 能讲到底层，我就觉得这本书写得值。

GitHub / Gitee Issues 见。

---

## 如何获取 Linux 内核源码做参考阅读

书里 ch13 / ch16 / ch17 + 附录 A 多处引用 Linux 内核 v6.6 LTS 的源码，给的都是相对内核仓库根的路径，比如 `include/linux/fs.h`、`drivers/gpio/gpiolib.c`、`init/main.c`。

要在本地打开查看，先 clone 一份内核源码：

```bash
git clone --depth=1 --branch v6.6 https://github.com/torvalds/linux.git linux-kernel
```

国内访问 GitHub 慢，用清华镜像更快：

```bash
wget https://mirrors.tuna.tsinghua.edu.cn/kernel/v6.x/linux-6.6.tar.xz
tar -xJf linux-6.6.tar.xz
```

Linux 内核源码 1.5 GB+，不会随这本书一起发布。clone 到任何方便位置即可。之后书里写"`include/linux/fs.h` 第 1852 行"，进 `linux-kernel/` 或 `linux-6.6/` 目录就能直接打开看。

Windows 用户注意：Linux 内核里有少数文件名（如 `drivers/gpu/drm/nouveau/nvkm/subdev/i2c/aux.c`）触发 NTFS 保留设备名，git checkout 会报错。可以用 sparse-checkout 跳过 `drivers/gpu/`，或者用 WSL / Linux 虚拟机 clone。书里引用的源码都不在 `drivers/gpu/` 目录下，跳过它对阅读完全没影响。

## 下一篇

[尾声 · 致读者](../尾声-致读者.md)
