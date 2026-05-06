# 5 分钟看见你的第一个 OOP LED

读理论之前，先把代码跑起来。

费曼讲过一句话：被自己说服，才叫理解。这本书的写法是每行代码都讲到不开 IDE 也能 follow，但如果你愿意花 5 分钟跑一次，"哦原来是这样"会变成"原来真的是这样"，印象深一倍。

这一章不解释任何概念，只做一件事：让你在 5 分钟内跑通一段 C 代码，看到屏幕输出三颗 LED 被同一份函数依次点亮。

## 准备工作

需要的工具只有一个：GCC 编译器。

Windows 用户装 MinGW 或 MSYS2 任选一个（搜官方安装包一路 next，勾选"添加到 PATH"），然后命令行敲 `gcc --version` 看到版本号即装好。

Linux 用户：`sudo apt install gcc make`（Debian/Ubuntu）或 `sudo yum install gcc make`（RHEL/CentOS）。

完全不想装环境也行：每个章节代码包都附带预编译好的 `demo.exe`，Windows 双击即可看到完整输出。

## 三步跑通

第一步，克隆仓库：

```bash
git clone https://github.com/ZhaoChengBo/zhaoming-embedded.git
cd zhaoming-embedded/oop-in-c/code/01-three-leds/pc
```

国内访问 GitHub 慢可以用 Gitee：

```bash
git clone https://gitee.com/zhao-chengbo/zhaoming_embedded.git
cd zhaoming_embedded/oop-in-c/code/01-three-leds/pc
```

第二步，编译：

```bash
make
```

或者直接用 GCC：

```bash
gcc -Wall -Wextra -std=c99 -I../../common -o demo main.c led.c ../../common/platform_pc.c
```

编译成功，当前目录会多出一个 `demo`（Linux/Mac）或 `demo.exe`（Windows）。

第三步，运行：

```bash
./demo
```

## 看到了什么

屏幕滚出几十行 `[GPIO]` 和 `[LED]` 输出。三颗 LED（Pin 13 红、Pin 14 绿、Pin 15 蓝）依次被初始化、点亮、熄灭、调亮度。

打开 `main.c`，关键的几行是：

```c
struct led red_led;
struct led green_led;
struct led blue_led;

led_init(&red_led, 13);
led_init(&green_led, 14);
led_init(&blue_led, 15);

led_on(&red_led);
led_on(&green_led);
led_on(&blue_led);
```

`led_on()` 这个函数在 `led.c` 里只写了一份，不到 10 行。但它服务了三颗 LED。通过传不同的"挂号单"（`&red_led`、`&green_led`、`&blue_led`），它能服务无限多颗。

这就是封装。

你可能从来没听过这个词，也可能听过但觉得它玄。没关系。下一章从最朴素的"工程师都会的写法"开始（三颗 LED 写三份代码），一步步发现为什么必须演化成你刚才看到的样子。

## 跑不起来

| 报错 | 解决 |
|---|---|
| `gcc: command not found` | GCC 没装或不在 PATH，按"准备工作"那一节装一遍 |
| `make: command not found` | Windows 上 MinGW 装的是 `mingw32-make`，改名或建别名 |
| `fatal error: stdio.h: No such file` | GCC 装得不全，重装并选完整开发套件 |
| 中文路径报错 | 把仓库克隆到纯英文路径，如 `D:\code\zhaoming` |
| 编译过了但运行报错 | Windows 试 `./demo.exe`，或加 `./` 前缀 |

还是不行，到 [GitHub Issues](https://github.com/ZhaoChengBo/zhaoming-embedded/issues) 或 [Gitee Issues](https://gitee.com/zhao-chengbo/zhaoming_embedded/issues) 提一个，附系统、GCC 版本、完整报错。

## 为什么先看见再谈理论

接下来 ch01-ch20 共 20 章（OOP 主体 18 章 + 工业实战 2 章）+ 4 附录，你会一步步推出整个面向对象。先把这本书最终要让你写出的形态摆在面前，后面每一步演化你都有锚点对照。

三颗 LED 一份代码这件事不是我吹的。这本书 18 章 OOP 主体 + 工业实战 2 章要做的，就是让你心服口服地说"哦原来是这样推出来的"。

下一篇：[第 1 章 · 三个 LED 三份代码](../01-封装/01-三个LED三份代码.md)
