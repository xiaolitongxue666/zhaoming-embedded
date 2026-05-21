# ch13 · container_of

## 在线阅读

[container_of](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/13-container_of.html)

## 学习目标

- `offsetof` + 指针运算\n- 回调只拿 base 指针时的向下转型

## 需手写文件（建议）

`container_of` 宏 + led 子类文件

## 验收

从成员指针正确得到包含它的结构体

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/13-container-of/`](../../../../oop-in-c/code/13-container-of/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
