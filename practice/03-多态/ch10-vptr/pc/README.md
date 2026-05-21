# ch10 · ops 放进对象

## 在线阅读

[ops 放进对象](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/10-ops放进对象.html)

## 学习目标

- base 内含 `const struct led_ops *ops`\n- dispatch: `ops->on(me)`

## 需手写文件（建议）

`led_base.*` 及子类文件

## 验收

`led_on` 内部走 ops 表

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/10-vptr/`](../../../../oop-in-c/code/10-vptr/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
