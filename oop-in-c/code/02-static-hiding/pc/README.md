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

`led_create()` 返回一个 `struct led *`，main.c 通过这个不透明指针调
`led_on / led_off / led_set_brightness / led_get_state / led_destroy`
做完整生命周期。

任意一行 `red->pin = 999;` 取消注释，编译就过不去：

```
error: invalid use of undefined type 'struct led'
```

字段定义在 `led.c` 内部，`led.h` 只有 `struct led;` 这个 forward
declaration。外部代码根本不知道字段长什么样，自然改不了。

## Windows 用户

跑一次 `make` 就有 `demo.exe`。最后一行 `Press Enter to exit...` 让
双击运行也能看完整输出。

## 配套章节

[第 2 章 · 同事改了一行 LED 全乱了](../../../book/01-封装/02-同事改了一行.md)
