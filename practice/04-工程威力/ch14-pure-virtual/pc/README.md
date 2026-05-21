# ch14 · 纯虚与抽象类

## 在线阅读

[纯虚与抽象类](https://zhaochengbo.github.io/zhaoming-embedded/04-工程威力/14-纯虚与抽象类.html)

## 学习目标

- 未实现 op 的 ENOSYS 策略\n- 板级 `sensor_board_init`

## 需手写文件（建议）

抽象 ops + 板级 init

## 验收

调用未实现 op 时有明确失败路径

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/14-pure-virtual/`](../../../../oop-in-c/code/14-pure-virtual/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
