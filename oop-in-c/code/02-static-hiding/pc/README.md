# pc/ — PC 模拟版

不需要任何开发板，gcc 一句编译就能看到效果。

## 编译运行

```bash
make
./demo
```

或者一行 gcc：

```bash
gcc -Wall -Wextra -std=c99 -I../../common -o demo main.c led.c ../../common/platform_pc.c
```

## 看到什么

`struct led` 实例直接放栈上，`led_init(&red, 13)` 初始化，调
`led_on / led_off / led_set_brightness / led_get_state / led_deinit`
做完整生命周期。

字段在 `led.h` 公开，每个挂 `/* private */` 注释。`led.c` 内部的
`update_hardware` / `brightness_valid` 加 `static`，外部 `.c` 看不到。

试一下两层强度对比：

- 在 main.c 取消注释 `red.pin = 999;`，编译能过，纪律层拦
- 在 main.c 取消注释 `update_hardware(&red);`，链接器报
  `undefined reference to update_hardware`，机制层拦

## Windows 用户

跑一次 `make` 就有 `demo.exe`。最后一行 `Press Enter to exit...` 让
双击运行也能看完整输出。

## 配套章节

[第 2 章 · 同事改了一行 LED 全乱了](../../../book/01-封装/02-同事改了一行.md)
