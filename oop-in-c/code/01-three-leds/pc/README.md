# pc/ — PC 模拟版

不需要任何开发板，gcc 一句编译就能看到三颗 LED 的逻辑效果。

## 编译运行

```bash
make
./demo
```

或者一行 gcc：

```bash
gcc -Wall -Wextra -std=c99 -I../../common -o demo main.c led.c ../../common/platform_pc.c
```

## 看到的输出

每个 GPIO 操作打一行 `[GPIO] PA.13 -> HIGH (ON)`，每个 LED 操作打一行 `[LED] PA.13 ON`。三颗 LED (PA.13 / PA.14 / PA.15) 依次被同一份 `led_on()` 点亮，传不同的 `&red_led / &green_led / &blue_led` 指针即可。

## Windows 用户

跑一次 `make` 就有 `demo.exe`。直接双击运行，最后一行 `Press Enter to exit...` 让你能看完整输出。

## 配套章节

[第 1 章 · 三个 LED 三份代码](../../../book/01-封装/01-三个LED三份代码.md)
