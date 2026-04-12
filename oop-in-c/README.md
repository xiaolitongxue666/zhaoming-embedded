# C语言·一个LED讲透面向对象

> 从struct到Linux内核，用一个LED驱动讲透面向对象。

---

## 系列简介

这是一套用**C语言实现面向对象**的系列教程。不是理论课，是从一个LED驱动出发，一步步写出工业级代码。

**你会学到：**
- 封装、继承、多态在C语言中怎么实现
- Linux内核的struct、函数指针、container_of到底在干什么
- 为什么你天天用的HAL库就是OOP

**学完后：**
- 你能看懂HAL库源码
- 你能看懂Linux内核驱动的基本框架
- 你用C写的代码，和C++的class本质上是一回事

---

## 学习路径

| 期数 | 主题 | 核心概念 | 金句 |
|------|------|---------|------|
| **EP06** | 三个LED你写了三份代码 | struct + me指针 + 多实例 | 封装不是藏代码，是让同一份逻辑服务不同的数据 |
| **EP07** | 你同事改了一行代码LED全乱了 | static函数 + 头文件 = private + public | 信息隐藏不是不信任——是锁上门，谁都不容易犯错 |
| **EP08** | 你用C手搓了一个class | 函数前缀 + init/deinit = 类名 + 构造/析构 | C语言没有class？你天天都在写 |
| **EP09** | 你的全局变量，该死了 | static变量 + const + 数据归位 | 数据没有主人，bug就是主人 |
| **EP10** | HAL库几千个函数就一个套路 | 源码验证：你学的就是工业标准 | 几千个函数——就这一招 |
| EP11 | 三种LED一半代码重复 | struct嵌套 = 继承 | *待更新* |
| EP12 | 一个函数也能当参数传？ | 函数指针 | *待更新* |
| EP13 | 把函数指针装进一个盒子 | ops结构体 = vtable | *待更新* |
| EP14 | 能不能让代码自己知道该调谁？ | 向上转型 + 多态dispatch | *待更新* |
| EP15 | 从LED到Linux | container_of宏 | *待更新* |
| EP16 | 三千万行代码的秘密武器 | Linux内核OOP全貌 | *待更新* |

---

## 目录结构

```
oop-in-c/
├── code/
│   ├── common/              ← 平台抽象层（PC模拟GPIO）
│   ├── EP06_封装/           ← 每个EP是一个完整的学习包
│   │   ├── led.h / led.c   ← 示例代码
│   │   ├── main.c           ← 演示程序（双击demo.exe可直接运行）
│   │   ├── demo.exe         ← 编译好的可执行文件
│   │   ├── EP06_封装.pdf    ← 学习文档（设计说明+视频讲义+代码解读）
│   │   └── Makefile
│   ├── EP07_信息隐藏/
│   ├── EP08_手搓class/
│   ├── EP09_数据归位/
│   └── EP10_HAL映射/
└── README.md                ← 本文件（系列导航）
```

每个EP目录就是一个**完整的学习包**——打开文件夹，看PDF学概念，看代码学实现，双击exe看效果。

---

## 如何运行

### 最简单的方式：直接双击

每个EP目录里都有编译好的 `demo.exe`（Windows），**双击即可运行**，无需安装任何工具。

程序运行完会显示 `Press Enter to exit...`，按回车关闭。

### 想自己编译？

环境要求：**GCC**（MinGW、MSYS2、Linux自带gcc均可），不需要开发板。

```bash
# EP06 示例
cd oop-in-c/code/EP06_封装
gcc -Wall -Wextra -I../common -o demo main.c led.c ../common/platform_pc.c
./demo

# EP07 示例
cd oop-in-c/code/EP07_信息隐藏
gcc -Wall -Wextra -I../common -o demo main.c led.c ../common/platform_pc.c
./demo

# EP08 示例（两个模块）
cd oop-in-c/code/EP08_手搓class
gcc -Wall -Wextra -I../common -o demo main.c led.c motor.c ../common/platform_pc.c
./demo

# EP09 示例（含反面教材）
cd oop-in-c/code/EP09_数据归位
gcc -Wall -Wextra -I../common -o demo main.c led.c led_bad.c ../common/platform_pc.c
./demo

# EP10 示例（HAL风格，不需要common）
cd oop-in-c/code/EP10_HAL映射
gcc -Wall -Wextra -o demo main.c hal_gpio.c
./demo
```

### 有STM32开发板？

代码通过`platform.h`抽象了GPIO操作。如果你有开发板：
1. 写一个`platform_stm32.c`，实现同样���接口
2. 编译时用你的`platform_stm32.c`替换`platform_pc.c`
3. 上层代码一行不改——这就是��台抽象的威力

---

## C vs C++ 对照表（全系列）

| C语言写法 | C++写法 | 首次出现 |
|----------|---------|---------|
| struct + me指针 | class + this | EP06 |
| static + .h | private + public | EP07 |
| 函数前缀 + init/deinit | 类名 + 构造/析构 | EP08 |
| file-scope static变量 | class static成员 | EP09 |
| struct嵌套 | class继承 | EP11 |
| 函数指针 | std::function / lambda | EP12 |
| ops结构体 | vtable虚函数表 | EP13 |
| ops dispatch | virtual虚函数 | EP14 |
| container_of | dynamic_cast（近似） | EP15 |

---

## 获取更多

- 公众号搜索 **「兆鸣嵌入式」** 获取完整学习路线
- 回复「交流」加技术交流群
