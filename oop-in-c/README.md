# C 语言一个 LED 讲透面向对象（代码包）

[《C 语言面向对象编程·嵌入式实战》](../book/README.md) 一书的配套代码包。

每个 EP 子目录对应书中的一章 + 一期视频。代码经过持续维护，每次提交都过编译。

## 17 期对应 17 章

| 期 | 主题 | 核心概念 | 书章节 |
|---|---|---|---|
| EP06 | 三个 LED 你写了三份代码 | struct + me 指针 | [ch1](../book/01-封装/01-三个LED三份代码.md) |
| EP07 | 你同事改了一行你的 LED 全乱了 | static + 信息隐藏 | ch2 |
| EP08 | 你用 C 手搓了一个 class | 函数前缀 + init/deinit | ch3 |
| EP09 | 你的全局变量该死了 | static 变量 + const + 数据归位 | ch4 |
| EP10 | HAL 库几千个函数就一个套路 | 源码验证（验证课） | ch5 |
| EP11 | 你的代码一半是重复的 | struct 嵌套 = 继承 | ch6 |
| EP12 | 写死的函数怎么换 | 函数指针 | ch7 |
| EP13 | 把号码给别人拨 | 函数指针传参 | ch8 |
| EP14 | 参数长到换行 | ops 表 = vtable | ch9 |
| EP15 | ops 放进对象 | 绑定 = vptr | ch10 |
| EP16 | 同名函数不同行为 | dispatch = vcall | ch11 |
| EP17 | 一个指针指所有 LED | 向上转型 | ch12 |
| EP18 | container_of 的地址魔法 | 向下转型 | ch13 |
| EP19 | 虚函数不实现会怎样 | 纯虚 + 合同防御 | ch14 |
| EP20 | 300 行砍到 60 行 | 完整框架 | ch15 |
| EP21 | 为什么 Linux 一点都不难 | Platform 层 | ch16 |
| EP22 | 4000 万行一招写完 | 内核映射 | ch17 |

## 学完之后

看懂 HAL 库和 Linux 内核驱动的骨架。用 C 写出和 C++ class 等价的代码。换芯片或换 LED 类型时只改驱动文件，应用层一行不动。在面试里把 `container_of`、函数指针、平台抽象讲到底层原理。

## 目录

```
oop-in-c/
├── README.md
└── code/
    ├── common/
    │   ├── platform.h          平台抽象层接口
    │   └── platform_pc.c       PC 模拟实现
    ├── EP06_封装/
    │   ├── led.h / led.c
    │   ├── main.c
    │   ├── Makefile
    │   ├── demo.exe            预编译的 Windows 可执行
    │   └── EP06_封装.pdf       早期版本文档，最新内容看在线书
    ├── EP07_信息隐藏/
    ├── EP08_手搓class/
    ├── EP09_数据归位/
    ├── EP10_HAL映射/
    ├── EP11_提公因式/
    ├── EP12_函数指针/
    ...
    └── EP22_终章/
```

每个 EP 目录是一个完整的学习包——打开文件夹，看在线书学概念，看代码学实现，双击 exe 看效果。

EP11 之后的代码包正在按视频内容补齐中。

## 怎么跑

最简单：Windows 直接双击 `demo.exe`，无需任何工具。运行完会显示 `Press Enter to exit...`，按回车关闭。

自己编译需要 GCC（MinGW、MSYS2、Linux 自带 gcc 均可），不需要开发板：

```bash
cd oop-in-c/code/EP06_封装
make
./demo
```

或者直接用 GCC：

```bash
gcc -Wall -Wextra -I../common -o demo main.c led.c ../common/platform_pc.c
./demo
```

环境配置看 [setup/README.md](../setup/README.md)。

## 有 STM32 开发板

代码通过 `platform.h` 抽象了 GPIO 操作。如果有开发板：写一个 `platform_stm32.c` 实现 `platform.h` 同样的接口；编译时用 `platform_stm32.c` 替换 `platform_pc.c`；上层代码一行不改。

这就是平台抽象的威力，第 16 章会展开讲。

## C 与 C++ 全系列对照

| C 写法 | C++ 写法 | 首次出现 |
|---|---|---|
| `struct` + me 指针 | `class` + `this` | EP06 / ch1 |
| `static` + `.h` | `private` + `public` | EP07 / ch2 |
| 函数前缀 + init/deinit | 类名 + 构造/析构 | EP08 / ch3 |
| file-scope `static` 变量 | `class` static 成员 | EP09 / ch4 |
| `struct` 嵌套 | `class` 继承 | EP11 / ch6 |
| 函数指针 | `std::function` / lambda | EP12 / ch7 |
| ops 结构体 | vtable 虚函数表 | EP14 / ch9 |
| ops dispatch | virtual 虚函数 | EP16 / ch11 |
| `container_of` | `dynamic_cast`（近似） | EP18 / ch13 |
| Platform 层 | 抽象基类 + 工厂 | EP21 / ch16 |
