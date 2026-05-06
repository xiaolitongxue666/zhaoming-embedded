# pc/ — PC 模拟版

不需要任何开发板，gcc 一句编译就能看到效果。

## 编译运行

```bash
make
./demo
```

## 看到什么

`Part 1` 跑 `led_bad.c`：5 个全局变量满天飞，两个 LED 共享 `g_pin`，
第二次 `bad_led_init` 覆盖第一次的引脚号，"红灯" 操作的实际是绿
灯的引脚。bug 的全过程一目了然。

`Part 2` 跑 `led.c`：数据归位完成形态。两个 LED 各有自己的 `pin /
brightness / is_on` 字段，从静态对象池 `led_pool[8]` 取槽，互不干扰。
模块级数据 `s_init_count` 走 `led_get_init_count()` 函数访问，外部
看不到这个变量。

最后演示对象池耗尽：连续 acquire 9 次，第 9 次返回 NULL，模块拒
绝继续分配。

## Windows 用户

跑一次 `make` 就有 `demo.exe`。最后一行 `Press Enter to exit...`
让双击运行也能看完整输出。

## 配套章节

[第 4 章 · 你的全局变量该死了](../../../book/01-封装/04-数据归位.md)
