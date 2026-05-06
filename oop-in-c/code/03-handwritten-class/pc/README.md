# pc/ — PC 模拟版

不需要任何开发板，gcc 一句编译就能看到效果。

## 编译运行

```bash
make
./demo
```

## 看到什么

`led_init / led_on / led_set_brightness / led_deinit` 和
`motor_init / motor_start / motor_set_speed / motor_set_direction /
motor_deinit` 在同一个 main 里和平共处。前缀让两套同名概念
（init / on / off）不撞车。

`initialized` 标志位拦截"没初始化就用"的常见错误：用未初始化的
`struct motor` / `struct led` 调操作函数，函数返回 -3 而不是去
操作未配置的 GPIO。

参数超范围（pin 200，duty 250）API 立即返回 -2，不会写到硬件。

## Windows 用户

跑一次 `make` 就有 `demo.exe`。最后一行 `Press Enter to exit...`
让双击运行也能看完整输出。

## 配套章节

[第 3 章 · 你用 C 手搓了一个 class](../../../book/01-封装/03-手搓class.md)
