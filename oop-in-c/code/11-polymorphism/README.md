# 11-polymorphism — 父类统一接口 led_on / led_off / led_toggle

第 11 章 [同名函数不同行为](../../../book/03-多态/11-多态完整图景.md) 的配套代码。

## 演化点

把 ch10 应用层那行 `me->ops->on(me)` 包装成 `led_on(base)`，写在父类 `led_base.c` 里，所有子类共用。应用层只调 `led_on / led_off / led_toggle`，看不到 ops 字段，也不需要知道这颗 LED 是 GPIO 还是 PWM 还是 I2C。

```c
/* 父类统一接口 - 写在 led_base.c, 函数体一行 */
int led_on(struct led_base *me) { return me->ops->on(me); }

/* 应用层: 三颗 LED 装进 base 指针数组, 一行 led_on(base) 跑出三种行为 */
struct led_base *all_leds[] = { &red.base, &blue.base, &green.base };
for (int i = 0; i < 3; ++i)
	led_on(all_leds[i]);
```

红灯落到 `gpio_on`，蓝灯落到 `pwm_on`，绿灯落到 `i2c_on`。同名函数不同行为，这就是多态。

## 目录

```
11-polymorphism/
├── pc/                          完整可跑 PC 模拟版
│   ├── Makefile
│   ├── led_base.h / .c          父类字段集 + 父类统一接口 led_on/off/toggle
│   ├── led_gpio.h / .c          GPIO 子类 + ops 表
│   ├── led_pwm.h  / .c          PWM 子类  + ops 表
│   ├── led_i2c.h  / .c          I2C 子类  + ops 表
│   └── main.c                   base 指针数组循环演示
└── platform-mcu/
    └── stm32/                   STM32 真机版片段（用 PIN_NUM 编码，每个子类一个文件）
        ├── led_gpio.c           GPIO 子类 + platform_gpio_* (HAL_GPIO_*)
        ├── led_pwm.c            PWM  子类 (HAL_TIM_PWM_* + __HAL_TIM_SET_COMPARE)
        └── led_i2c.c            I2C  子类 (HAL_I2C_Master_Transmit)
```

`platform-mcu/stm32/` 是参考片段，不参与 PC build。

## PIN 编码

`platform-mcu/stm32/led_gpio.c` 用一个 `uint8_t pin` 同时表示 port 和 pin 号，`PIN_NUM('A', 13) = 0x0D`。共享头 `oop-in-c/code/common/platform.h`。pc 版的 GPIO 模拟实现来自 `oop-in-c/code/common/platform_pc.c`，跟 ch01 起一字不变。

## 编译运行

```bash
cd pc
make
./demo
```
