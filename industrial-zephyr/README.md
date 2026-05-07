# industrial-zephyr

zhaoming-embedded 在线书 **ch19 (Zephyr 实战) + 附录 B (Zephyr 完整工程)** 的配套代码骨架。

## 这是什么

一份 freestanding Zephyr application·**不是 west workspace**。读者自己装好 Zephyr SDK 后用环境变量指过来·这份工程跑 4 个 demo 验证书里讲的 4 个 OOP 主题。

| Demo | 对应章节 | 主题 | 教学呼应 |
|------|----------|------|----------|
| 1 | ch19/19.7 | 4 LED 同 ops 表跑 | ch12 向上转型 |
| 2 | ch19/19.8 | overlay 改 label·应用层零改动 | ch15 dts 解耦 |
| 3 | ch19/19.9 | CONTAINER_OF 抓回调 | ch13/ch18 |
| 4 | ch19/19.10 | 可空 ops·返回 -ENOSYS | ch14/ch16 |

## 目录结构

```
industrial-zephyr/
├── README.md
├── CMakeLists.txt              # -DDEMO=N 切换 4 个 main
├── prj.conf                    # 4 个 demo 公用
├── boards/
│   └── stm32f4_disco.overlay   # demo2 用·只改 label
└── src/
    ├── main_demo1_4led.c
    ├── main_demo2_overlay.c
    ├── main_demo3_container.c
    └── main_demo4_enosys.c
```

## 前置条件

1. **Zephyr v3.7.0 LTS**·按 [Zephyr Getting Started](https://docs.zephyrproject.org/3.7.0/develop/getting_started/index.html) 装好 west workspace
2. **Zephyr SDK v0.16.x**·至少装 arm-zephyr-eabi
3. 设好环境变量：

   ```bash
   export ZEPHYR_BASE=/path/to/zephyrproject/zephyr
   ```

   Windows PowerShell：

   ```powershell
   $env:ZEPHYR_BASE = "C:\zephyrproject\zephyr"
   ```

## Build & Flash

每个 demo 都用 `-DDEMO=N` 切换。**注意 `--` 后面的才是传给 CMake 的参数·-p auto 让 west 在切换 demo 时自动 pristine 重 build。**

```bash
# Demo 1: 4 LED 同 ops 表跑
west build -b stm32f4_disco -p auto -- -DDEMO=1
west flash

# Demo 2: overlay 改 label
west build -b stm32f4_disco -p auto -- -DDEMO=2
west flash

# Demo 3: CONTAINER_OF 抓回调
west build -b stm32f4_disco -p auto -- -DDEMO=3
west flash

# Demo 4: 可空 ops 看 -ENOSYS
west build -b stm32f4_disco -p auto -- -DDEMO=4
west flash
```

## 期望输出

串口波特率 115200·8N1。

### Demo 1

```
[demo1] 4 LED 同 ops 表跑·按编号 0/1/2/3 顺次点亮
```

板上 LD3/LD4/LD5/LD6 顺次亮灭。

### Demo 2

```
[demo2] led0 label: Demo Board LD4 (overlay)
[demo2] 应用层一行没改·只是 boards/stm32f4_disco.overlay 改了 label
```

把 `boards/stm32f4_disco.overlay` 删掉再 build·label 会变回 dts 默认值。两次对比说明 overlay 生效。

### Demo 3

```
[demo3] CONTAINER_OF 抓回调演示开始
[demo3] offsetof(ctx, timer) = 0
[demo3] tick=1 led=0  ctx=0x... timer=0x... offset=0
[demo3] tick=2 led=0  ctx=0x... timer=0x... offset=0
...
```

LD3 闪烁·log 里 ctx 和 timer 地址相同·因为 timer 是 ctx 的第一字段·offsetof = 0。把 timer 移到第二个字段重新 build·offset 就会变成 sizeof 前面那个字段。

### Demo 4

```
[demo4] 可空 ops 演示·调没实现的动作看返回 -ENOSYS
[demo4] led_blink returned -88  (期望 -88 即 -ENOSYS)
[demo4] led_set_brightness returned 0  (期望 0)
[demo4] led_on returned 0  (期望 0)
```

`-88` 就是 `-ENOSYS`·led_gpio 子类没实现 blink·led 子系统直接返回。

## 故障排查

- **`Zephyr package not found`**: `ZEPHYR_BASE` 没设·或者指错了路径
- **`board stm32f4_disco not found`**: zephyr v3.7.0 之前板子目录在 `zephyr/boards/arm/stm32f4_disco/`·v3.7.0 改成了 `zephyr/boards/st/stm32f4_disco/`·两份都行 west 会自己搜
- **链接报错 `region FLASH overflowed`**: 把 prj.conf 里 `CONFIG_LOG=y` 临时改成 n·LOG 子系统占 flash 不少
- **demo2 看到的 label 还是旧的**: `-p auto` 漏了·改成 `-p always` 强制 pristine

## 红线

- 所有源码 `SPDX-License-Identifier: Apache-2.0`
- 中文注释·英文术语保留原形
- 不要 typedef `_t` (除合法的 platform 已有类型)
- 内存安全：本工程主要靠 `DEVICE_DT_GET` + 静态实例·没有动态分配
