# ch09 · ops 操作表

## 在线阅读

[ops 操作表](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/09-ops操作表.html)

## 学习目标

- `struct led_ops` 函数表\n- `led_ops_gpio` / `led_ops_pwm` 填充

## 需手写文件（建议）

ops 表相关 `.h/.c`

## 验收

换 ops 表即换实现，main 不直接调具体驱动函数

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/09-ops-table/`](../../../../oop-in-c/code/09-ops-table/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
