# ch13 · container_of 的地址魔法 · 向下转型

配套书章节：[`book/04-工程威力/13-container_of.md`](../../../book/04-工程威力/13-container_of.md)

## 看点

- `container_of(ptr, type, member)` 的 PC 友好实现（见 `pc/container_of.h`）
- GPIO 子类故意把 base 放到第二个位置（前面挡了一个 `magic` 字段），证明 container_of 与 base 在 struct 里的位置无关
- 跑出来打印 `offsetof(struct led_gpio, base) = 4`，但每次操作仍能正确还原 magic、pin、on_level

## 目录

```
pc/                              完整可跑 PC 模拟版
├── led.h                        父类 + 子类声明
├── led_base.h / .c              父类
├── led_gpio.h / .c              GPIO 子类（base 在中间）
├── led_pwm.h  / .c              PWM 子类
├── led_i2c.h  / .c              I2C 子类
├── container_of.h               container_of 宏定义
├── main.c                       offsetof 打印 + 三个子类 toggle
└── Makefile                     引用 ../../common/platform_pc.c

platform-mcu/
└── stm32/                       STM32 真机版片段（用 PIN_NUM 编码，每个子类一个文件）
    ├── led_gpio.c               GPIO 子类（container_of 反推）+ platform_gpio_*
    ├── led_pwm.c                PWM  子类（container_of 反推） (HAL_TIM_PWM_*)
    └── led_i2c.c                I2C  子类（container_of 反推） (HAL_I2C_Master_Transmit)
```

`platform-mcu/stm32/` 是参考片段，不参与 PC build。

ch11+ 的 ops 表式 platform 抽象在 ch15 重构展开。

## 跑一遍

```
cd pc
make
./demo
```
