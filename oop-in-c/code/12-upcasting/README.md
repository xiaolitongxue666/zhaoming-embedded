# ch12 · 一个指针指所有 LED · 向上转型

配套书章节：[`book/04-工程威力/12-向上转型.md`](../../../book/04-工程威力/12-向上转型.md)

## 目录

```
pc/                  完整可跑 PC 模拟版
├── led.h, led.c           父类 led_base + 子类 led_gpio/led_pwm/led_i2c
├── leds.h                 板级对外暴露的全局句柄
├── board_init.c           唯一认识硬件的文件
├── main.c                 应用层（零硬件字样）
├── platform_ops.h         本章自带 platform_ops 接口
├── platform_ops_pc.c      PC 模拟实现
└── Makefile

stm32-snippet/       STM32 HAL 等效片段（替换 platform_ops_pc.c 即可）
└── platform_ops_stm32.c

linux-snippet/       Linux 用户态等效片段（sysfs）
└── platform_ops_linux.c
```

## 跑一遍

```
cd pc
make
./demo
```

## 看点

- `struct led_base` 在三个子类（`led_gpio` / `led_pwm` / `led_i2c`）的第 0 偏移
- `g_led_error / g_led_status / g_led_network` 三个 `struct led_base *` 全局句柄
- `board_init` 里把子类对象的 `&xxx.base` 赋给句柄，这就是向上转型的工程化落地
- `main.c` 里 `grep gpio_write`、`grep pwm_`、`grep i2c_` 全部 0 个

应用层不认识硬件，硬件是谁它都不问。
