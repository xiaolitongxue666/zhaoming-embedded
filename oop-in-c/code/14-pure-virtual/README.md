# ch14 · 虚函数不实现 · 三种策略

配套书章节：[`book/04-工程威力/14-纯虚与抽象类.md`](../../../book/04-工程威力/14-纯虚与抽象类.md)

## 看点

- `led_on / led_off`：必填（C++ 纯虚函数对应物）。子类 ops 没填，父类的统一接口里 `assert` 失败。
- `led_set_brightness`：选填（C++ 虚函数对应物）。子类没填，父类的统一接口走默认行为，安静返回。
- `sensor_read / calibrate / self_test`：全必填（C++ 接口对应物）。三个 op 全部 assert，少一个就报错。

LED 体系沿用前面章节的"父类通用 init 接 ops"风格：子类 init 第一行调 `led_base_init(&me->base, name, &xxx_ops)`，把对应的 const ops 表作为常量交给 base，一次填好。GPIO 子类故意只填 on / off，演示选填策略；PWM 子类三件套全填。

sensor 体系单独一份父类（`sensor_base.h / .c`）+ 子类（`sensor.h / .c`），结构和 LED 体系完全一致，区别只在统一接口的三个函数全部走 assert，演示接口风格。

## 目录

```
pc/                              完整可跑 PC 模拟版
├── led.h                        父类 + 子类声明
├── led_base.h / .c              LED 父类（led_base_init + led_on/off/set_brightness）
├── led_gpio.h / .c              GPIO 子类（只填 on/off，演示选填）
├── led_pwm.h  / .c              PWM 子类（三件套全填）
├── sensor.h / .c                temp_sensor 子类
├── sensor_base.h / .c           sensor 父类（接口风格三件套全 assert）
├── container_of.h               container_of 宏定义
├── main.c                       三种策略演示
└── Makefile                     引用 ../../common/platform_pc.c

platform-mcu/
└── stm32/                       STM32 真机版片段（用 PIN_NUM 编码，每个子类一个文件）
    ├── led_gpio.c               GPIO 子类（只填 on/off）+ platform_gpio_*
    └── led_pwm.c                PWM  子类（三件套全填，HAL_TIM_PWM_*）
```

`platform-mcu/stm32/` 是参考片段，不参与 PC build。

本章主线是 led_ops 这一层（子类层）的三种策略，platform 只是稳定背景，所以这里 platform 用函数式包装即可。platform 层从函数式演化成 ops 表是 ch15 的主题。

## 跑

```
cd pc
make
./demo
```
