# practice · 个人手写练习区

本目录与在线书 [**《C 语言面向对象编程·嵌入式实战》**](https://zhaochengbo.github.io/zhaoming-embedded/) 章节一一对应，供你在 PC 上手写代码、编译验证，再对照仓库官方教学包 `oop-in-c/code/`。

## 怎么用

1. 打开 [`LEARNING_PLAN.md`](LEARNING_PLAN.md)，按阶段勾选进度。
2. 进入对应章节的 `pc/` 目录，阅读本章 `README.md`。
3. 先读在线书章节，**关闭参考代码**后手写 `.c` / `.h`。
4. 在 `Makefile` 的 `SRCS` 中追加你写的源文件，然后：

```bash
make
./demo
```

5. 0 警告通过后，再打开 `oop-in-c/code/<章目录>/` 对比差异。

## 平台层

各章 `Makefile` 已链接仓库共用的 [`oop-in-c/code/common/`](../oop-in-c/code/common/)（`platform.h` + `platform_pc.c`）。你只需手写 LED / 驱动等业务代码，不必每章重写 printf 模拟 GPIO。

## 环境

GCC + make。Windows 推荐 MSYS2，详见 [`setup/README.md`](../setup/README.md)。

验证环境：

```bash
cd practice/01-封装/ch01-three-leds/pc
make && ./demo
```

## 目录一览

| 部分 | 目录 | 章节 |
|------|------|------|
| 序曲 | `00-序曲/` | 阅读 + 预热 |
| 封装 | `01-封装/` | ch01–ch05 |
| 继承 | `02-继承/` | ch06 |
| 多态 | `03-多态/` | ch07–ch11 |
| 工程威力 | `04-工程威力/` | ch12–ch18 |
| 工业实战 | `05-工业实战/` | ch19–ch20（阅读 + 硬件工程） |

完整学习计划见 [`LEARNING_PLAN.md`](LEARNING_PLAN.md)。
