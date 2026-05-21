# ch06 · 代码一半重复 · 继承

## 在线阅读

[代码一半重复 · 继承](https://zhaochengbo.github.io/zhaoming-embedded/02-继承/06-代码一半重复.html)

## 学习目标

- `struct led_base` 作为首成员\n- 子类 init 先初始化父类部分

## 需手写文件（建议）

`led_base.*`、`led_gpio.*`、`led_pwm.*`

## 验收

子类复用父类字段，重复代码显著减少

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/06-inherit-pain/`](../../../../oop-in-c/code/06-inherit-pain/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
