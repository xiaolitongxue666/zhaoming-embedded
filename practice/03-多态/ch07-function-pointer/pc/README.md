# ch07 · 写死的函数怎么换

## 在线阅读

[写死的函数怎么换](https://zhaochengbo.github.io/zhaoming-embedded/03-多态/07-写死的函数怎么换.html)

## 学习目标

- 函数名 = 地址\n- `void (*fp)(int)` 切换实现

## 需手写文件（建议）

主要在 `main.c`（可加辅助 .c）

## 验收

切换 fp 后输出对应实现路径

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/07-function-pointer/`](../../../../oop-in-c/code/07-function-pointer/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
