# ch08 · 把号码给别人拨

## 在线阅读

[把号码给别人拨](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/08-把号码给别人拨.html)

## 学习目标

- 函数指针作为参数传递\n- 注册底层实现

## 需手写文件（建议）

`led.h`、`led.c`

## 验收

注册不同回调后 LED API 行为一致

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/08-callback/`](../../../../oop-in-c/code/08-callback/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
