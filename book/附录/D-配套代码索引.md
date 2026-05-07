# 附录 D · 配套代码索引

参考代码：
- 教学包：[`oop-in-c/code/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/oop-in-c/code/) · 前 18 章逐章配套，每章一个独立目录，PC 上直接 gcc 编译运行
- Zephyr 工程：[`industrial-zephyr/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/industrial-zephyr/) · ch19 + 附录 B 配套，stm32f4_disco 真机
- Linux 工程：[`industrial-linux/`](https://github.com/ZhaoChengBo/zhaoming-embedded/tree/master/industrial-linux/) · ch20 + 附录 C 配套，Raspberry Pi 4B 真机，含自写内核驱动 leds-status.c

## D.1 教学包 oop-in-c/code/

按章节组织，每章一个独立目录。前 18 章每个核心抽象都在 PC 上 demo，学生不需要任何嵌入式开发板，`gcc demo.c -o demo && ./demo` 直接看输出。

| 章 | 目录 | 演示的抽象 |
|---|---|---|
| ch01 | `01-three-leds/` | 三个 LED 三份代码（重复痛点） |
| ch02 | `02-static-hiding/` | static + 信息隐藏 |
| ch03 | `03-handwritten-class/` | 句柄 + 操作函数 |
| ch04 | `04-data-classification/` | 数据三级分类 |
| ch05 | `05-hal-mapping/` | HAL 映射 |
| ch06 | `06-inherit-pain/` | 共性提取的痛点 |
| ch07 | `07-function-pointer/` | 函数指针入门 |
| ch08 | `08-callback/` | 函数指针传参（callback） |
| ch09 | `09-ops-table/` | ops 操作表 |
| ch10 | `10-vptr/` | ops 放进对象（vptr 落地） |
| ch11 | `11-polymorphism/` | 多态完整图景 |
| ch12 | `12-upcasting/` | 向上转型 |
| ch13 | `13-container-of/` | container_of |
| ch14 | `14-pure-virtual/` | 纯虚 + 三种策略 |
| ch15 | `15-platform/` | platform 抽象 |
| ch16 | `16-linux-style/` | Linux 风格 |
| ch17 | `17-initcall/` | 链接自动初始化 |
| ch18 | `18-roadmap/` | 全书地图回顾 |

每个目录里都有 `README.md` + Makefile + .c 源 + demo.exe（学生可以双击直接跑）。

## D.2 Zephyr 工程 industrial-zephyr/

ch19 / 附录 B 配套，参考板 stm32f4_disco，配套 Zephyr v3.7.0 LTS。

```
industrial-zephyr/
├── README.md
├── CMakeLists.txt
├── prj.conf
├── boards/
│   └── stm32f4_disco.overlay
└── src/
    ├── main_demo1_4led.c       # ch19/19.7 · 4 颗 LED 同 ops 表跑
    ├── main_demo2_overlay.c    # ch19/19.8 · overlay 改 label
    ├── main_demo3_container.c  # ch19/19.9 · CONTAINER_OF 抓回调
    └── main_demo4_enosys.c     # ch19/19.10 · 可空 ops + ENOSYS
```

build：`west build -b stm32f4_disco -p auto -- -DDEMO=N`（N=1/2/3/4），flash：`west flash`。完整步骤见**附录 B**。

## D.3 Linux 工程 industrial-linux/

ch20 / 附录 C 配套，参考板 Raspberry Pi 4B，配套 Linux 6.6 主线内核。

```
industrial-linux/
├── README.md
├── ch20-leds-status/        # ch20/20.8 · 写自己的内核驱动 leds-status.c
│   ├── Makefile
│   ├── leds-status.c
│   ├── leds-status-overlay.dts
│   └── README.md
├── ch20-demo2-libgpiod/       # ch20/20.9 · sysfs vs libgpiod
├── ch20-demo3-gdb/            # ch20/20.10 · CONTAINER_OF 现场抓
└── ch20-demo4-initcall/       # ch20/20.11 · module_init 链路追踪
```

build / 跑命令完整步骤见**附录 C**。

## D.4 阅读路径建议

- 想跑通"动手验证"：先 ch01-ch18 教学包（PC 直接跑，零硬件门槛），然后 ch19 + 附录 B（stm32f4_disco），然后 ch20 + 附录 C（Raspberry Pi 4B）
- 想看"工业级 OOP 长什么样"：ch19/ch20 叙事 + Zephyr / Linux 上游源码（书里贴的 GitHub permalink）
- 想"自己写内核驱动"：直接跳 ch20/20.8 + 附录 C，5 分钟跑通

## D.5 上游源码 / 版本

- Zephyr v3.7.0 LTS：`https://github.com/zephyrproject-rtos/zephyr/tree/v3.7.0`
- Linux 6.6 stable：`https://github.com/torvalds/linux/tree/v6.6`

书里贴的所有代码段都来自这两个 tag，一年内升级新版后行号会变，**API 名 / 文件路径稳定**。
