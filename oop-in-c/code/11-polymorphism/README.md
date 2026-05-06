# 11-polymorphism — 父类统一接口 led_on / led_off / led_toggle

第 11 章 [同名函数不同行为](../../../book/03-多态/11-多态完整图景.md) 的配套代码。

## 演化点

把 ch10 应用层那行 `me->ops->on(me)` 包装成 `led_on(base)`，写在父类 `led_base.c` 里，所有子类共用。应用层只调 `led_on / led_off / led_toggle`，看不到 ops 字段，也不需要知道这颗 LED 是 GPIO 还是 PWM。

```c
/* 父类统一接口 - 写在 led_base.c, 函数体一行 */
int led_on(struct led_base *me) { return me->ops->on(me); }

/* 应用层: 三颗 LED 装进 base 指针数组, 一行 led_on(base) 跑出三种行为 */
struct led_base *all_leds[] = { &red.base, &blue.base, &green.base };
for (int i = 0; i < 3; ++i)
	led_on(all_leds[i]);
```

红灯落到 `gpio_on`，蓝灯落到 `pwm_on`，绿灯落到 `gpio_on`。同名函数不同行为，这就是多态。

## 文件清单

```
pc/
├── Makefile
├── led_base.h        - 父类字段集 (ops / name / is_on)
├── led_base.c        - 父类共有 init + 父类统一接口 led_on/off/toggle
├── led.h             - 子类声明 + ops 表声明
├── led.c             - 子类 init + 子类实现 (gpio_on / pwm_on / ...)
└── main.c            - base 指针数组循环演示

stm32-snippet/
└── led_stm32.c       - 真实 STM32 上的 platform_gpio_* 实现 (HAL)

linux-snippet/
└── led_linux.c       - 真实 Linux 用户态 platform_gpio_* 实现 (sysfs)
```

`stm32-snippet/` 和 `linux-snippet/` 是参考片段不参与 PC build。共享的 `oop-in-c/code/common/platform.h` + `platform_pc.c` 提供 PC 模拟版的 GPIO 操作。

## 编译运行

```bash
cd pc
make
./demo
```
