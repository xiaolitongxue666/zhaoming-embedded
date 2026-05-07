# ch14 · 虚函数不实现 · 三种策略

配套书章节：[`book/04-工程威力/14-纯虚与抽象类.md`](../../../book/04-工程威力/14-纯虚与抽象类.md)

## 看点

- `led_on / led_off`：必填（C++ 纯虚函数对应物）。子类 ops 没填，父类的统一接口里 `assert` 失败。
- `led_set_brightness`：选填（C++ 虚函数对应物）。子类没填，父类的统一接口走默认行为，安静返回。
- `sensor_read / calibrate / self_test`：全必填（C++ 接口对应物）。三个 op 全部 assert，少一个就报错。

LED 体系沿用前面章节的"父类通用 init 接 ops"风格：子类 init 第一行调 `led_base_init(&me->base, name, &xxx_ops)`，把对应的 const ops 表作为常量交给 base，一次填好。GPIO 子类故意只填 on / off，演示选填策略；PWM 子类三件套全填。

sensor 体系单独一份父类（`sensor_base.h / .c`）+ 子类（`sensor.h / .c`），结构和 LED 体系完全一致，区别只在统一接口的三个函数全部走 assert，演示接口风格。

## 文件清单

- `pc/led_base.h / .c`：LED 父类通用层（`led_base_init` + `led_on` / `led_off` / `led_set_brightness`）
- `pc/led.h / .c`：GPIO + PWM 子类
- `pc/sensor_base.h / .c`：sensor 父类通用层（`sensor_base_init` + 三件套接口）
- `pc/sensor.h / .c`：temp_sensor 子类
- `pc/main.c`：演示三种策略

## 跑

```
cd pc
make
./demo
```

## 目录

- `pc/` 完整可跑工程（PC 模拟 platform，printf 替代 GPIO）
- `stm32-snippet/platform_stm32.c` 把 platform 4 个函数换到 STM32 HAL，led.c / main.c 一字不改
- `linux-snippet/platform_linux.c` 把 platform 4 个函数换到 Linux 用户态 sysfs，led.c / main.c 一字不改

本章主线是 led_ops 这一层（子类层）的三种策略，platform 只是稳定背景，所以这里 platform 用函数式包装即可。platform 层从函数式演化成 ops 表是 ch15 的主题。
