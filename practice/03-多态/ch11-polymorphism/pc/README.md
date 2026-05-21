# ch11 · 多态完整图景

## 在线阅读

[多态完整图景](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/11-多态完整图景.html)

## 学习目标

- 完整子类集\n- 对外统一 `led_on` 胶水函数

## 需手写文件（建议）

全套 led 子类 + API

## 验收

同一 `led_on` 驱动不同子类实例

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/11-polymorphism/`](../../../../oop-in-c/code/11-polymorphism/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
