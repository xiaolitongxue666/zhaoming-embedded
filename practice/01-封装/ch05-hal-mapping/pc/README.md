# ch05 · HAL 映射

## 在线阅读

[HAL 映射](https://zhaochengbo.github.io/zhaoming-embedded/01-封装/05-HAL映射.html)

## 学习目标

- 抽象接口 → 平台实现\n- 对照 ST HAL 与本书 platform 层

## 需手写文件（建议）

以阅读笔记为主；可选 `notes.md` 或最小 platform 对照

## 验收

能口述应用 / 驱动 / HAL / 寄存器四层；`make` 通过即可

```bash
make
./demo
```

## 官方参考（验收后再看）

[`oop-in-c/code/05-hal-mapping/`](../../../../oop-in-c/code/05-hal-mapping/)

## 编译说明

- 平台层已链接 `oop-in-c/code/common/platform_pc.c`
- 手写业务代码后，编辑本目录 `Makefile` 的 `SRCS` 行
