# industrial-linux

zhaoming-embedded 在线书 **ch20 (Linux 实战) + 附录 C (Linux 完整工程)** 的配套代码骨架。

## 这是什么

四个独立 demo·串起来讲清楚 Linux 内核里的 OOP-in-C：从写一个 platform driver·到用 dts 解耦·到 gdb 验证 container_of·到 ftrace 看 initcall 链路。

## 目录结构

```
industrial-linux/
├── README.md                          # 本文件
├── ch20-leds-status/                # demo1 主菜·platform driver 三件套
│   ├── README.md
│   ├── Makefile
│   ├── leds-status.c
│   └── leds-status-overlay.dts
├── ch20-demo2-libgpiod/               # demo2 sysfs vs libgpiod 同硬件两接口
│   ├── Makefile
│   └── blink_libgpiod.c
├── ch20-demo3-gdb/                    # demo3 QEMU + gdb 验证 container_of 偏移
│   └── README.md
└── ch20-demo4-initcall/               # demo4 ftrace 看 initcall 链路
    └── trace_initcall.sh
```

## 4 个 demo 教学呼应

| Demo | 章节 | 主题 | 形态 |
|------|------|------|------|
| 1 | ch20.8 | platform driver 三件套：父类首字段 / container_of / module_platform_driver | C 源码 + dts |
| 2 | ch20.9 | sysfs vs libgpiod 同硬件两接口 | C 源码 |
| 3 | ch20.10 | QEMU + gdb·眼见为实看 container_of 偏移 = 0 | 文档 |
| 4 | ch20.11 | ftrace + dmesg·看 module_platform_driver 一行宏背后的 initcall 链路 | shell |

## 前置条件 (Raspberry Pi 4B 实机)

```bash
sudo apt update
sudo apt install raspberrypi-kernel-headers   # demo1 内核头
sudo apt install device-tree-compiler         # demo1 dts -> dtbo
sudo apt install libgpiod-dev gpiod           # demo2 用户态
sudo apt install qemu-system-arm gdb-multiarch # demo3 (host 装·不是 Pi 上)
```

## 走完一遍的推荐顺序

1. 进 `ch20-leds-status/`·`make` 编模块·编 overlay·`insmod` 验 sysfs
2. `rmmod` 模块·进 `ch20-demo2-libgpiod/`·`make` 编用户态·跑·**对比"两条路互斥"**
3. host 上看 `ch20-demo3-gdb/README.md`·QEMU + gdb 验 container_of 偏移
4. 回到模块·`bash ch20-demo4-initcall/trace_initcall.sh`·看 initcall 链路

## 红线

- 所有源码 `// SPDX-License-Identifier: GPL-2.0`
- 中文注释·英文术语保留 (gpiod / container_of / probe 等)
- 不要 typedef `_t`·内核风格直接用 `struct foo` 全名
- 内存安全：probe 全用 `devm_` 系列·错误路径单 return·不 goto unwind
- 错误返回值用 `-ENOMEM` / `PTR_ERR` / `ret`·不要 `1 / -1` 凑数
