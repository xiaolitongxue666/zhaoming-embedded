# ch15 · 换硬件不改应用 · OOP 完整框架

配套书章节：[`book/04-工程威力/15-Platform抽象.md`](../../../book/04-工程威力/15-Platform抽象.md)

## 看点

ch15 把整本书 ch01 - ch14 学过的所有 OOP 武器组装成一份完整 LED 框架，**0 个新概念**。四层架构：

- **父类层** `led.h` / `led.c`：`led_base` + `led_ops` + 必填 / 选填
- **子类层** `led.c`：`led_gpio` / `led_pwm` / `led_i2c`，每个子类第一行 `container_of` 反推
- **板级层** `leds.h` / `board_init.c`：三种子类混搭，向上转型挂全局句柄
- **应用层** `app.h` / `app.c`：`alarm_blink` / `status_indicate` / `power_on_test` 三个业务函数

主线：grep `app.c` 拿不到任何硬件字样（`LedGpio` / `LedPwm` / `LedI2c` / `gpio_write` 全部 0 命中）。换硬件方案改 `board_init.c` 三行，`app.c` 0 改动。

`pc/` 目录共 8 个文件，编译跑出 PC 模拟输出。STM32 / Linux 真机版只换底下 4 个 platform 封装函数（同样的签名），上面四层一字不动，对应 `stm32-snippet/` 和 `linux-snippet/`。

## 跑

```
cd pc
make
./demo
```

输出会看到：开机自检 → 报警闪烁 → 状态指示。三盏灯（GPIO+PWM+I2C 混搭）经过同一份 `led_on / led_off` 父类接口，分发到不同子类，落到 `platform_gpio_write` 等封装函数。

## 文件清单

```
pc/
├── main.c            主程序入口（单轮跑）
├── app.h, app.c      应用层 - 三个业务函数
├── leds.h            板级对外暴露的 g_led_xxx 句柄声明
├── board_init.c      板级 - 唯一认识硬件的文件
├── led.h, led.c      父类 + 三个子类
├── container_of.h    与 ch13 同款（最小可用版）
└── Makefile          链接 ../../common/platform_pc.c

stm32-snippet/led_stm32.c    真实 STM32 HAL 版的 4 个 platform 函数
linux-snippet/led_linux.c    真实 Linux 用户态 sysfs 版的 4 个 platform 函数
```
