# ch12 · 一个指针指所有 LED · 向上转型

配套书章节：[`book/04-工程威力/12-向上转型.md`](../../../book/04-工程威力/12-向上转型.md)

## 目录

```
pc/                              完整可跑 PC 模拟版
├── led_base.h / .c              父类
├── led_gpio.h / .c              GPIO 子类
├── led_pwm.h  / .c              PWM 子类
├── led_i2c.h  / .c              I2C 子类
├── leds.h                       板级对外暴露的全局句柄
├── board_init.c                 唯一认识硬件的文件
├── main.c                       应用层（零硬件字样）
└── Makefile                     引用 ../../common/platform_pc.c

platform-mcu/
└── stm32/                       STM32 真机版片段（用 PIN_NUM 编码，每个子类一个文件）
    ├── led_gpio.c               GPIO 子类 + platform_gpio_* (HAL_GPIO_*)
    ├── led_pwm.c                PWM  子类 (HAL_TIM_PWM_* + __HAL_TIM_SET_COMPARE)
    ├── led_i2c.c                I2C  子类 (HAL_I2C_Master_Transmit)
    └── board_init.c             STM32 板级 init（pin = PIN_NUM('A', 13)）
```

`platform-mcu/stm32/` 是参考片段，不参与 PC build。pc 版的 GPIO 模拟实现来自仓库共享的 `oop-in-c/code/common/platform_pc.c`，跟 ch01 起一字不变。

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
