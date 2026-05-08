# 08-callback — 把号码给别人拨

第 8 章 [把号码给别人拨](../../../book/03-多态/08-把号码给别人拨.md) 的配套代码。

## 主题

函数指针不只能存在一个变量里 (ch07)，还能当函数参数：

```c
void test_led(void (*on)(int), void (*off)(int), int id);
```

`test_led` 函数体不写死调谁。调用方把一对 `on`/`off` 函数指针 + 一个 `id`
传进来，`test_led` 就在内部代为拨号。同一个 `test_led`：

- 传 `gpio_on` / `gpio_off` + 引脚号 `15`   → 跑出 GPIO 的样子
- 传 `pwm_on` / `pwm_off` + 通道号 `3`      → 跑出 PWM 的样子
- 传 `i2c_on` / `i2c_off` + 地址 `0x50`     → 跑出 I2C 的样子

第三个参数 `id` 是个通用名：给 GPIO 当引脚号，给 PWM 当通道号，给 I2C
当从机地址。

## 文件清单

```
pc/
├── Makefile      gcc -Wall -Wextra -std=c99
├── led.h         test_led + 6 个占位函数声明
├── led.c         test_led 实现 + 6 个 printf 占位
└── main.c        三组调用演示

platform-mcu/
└── stm32/
    └── led_stm32.c   六个占位函数在 STM32 上的真实实现
                       (HAL_GPIO_WritePin / __HAL_TIM_SET_COMPARE /
                        HAL_I2C_Master_Transmit)
```

## 编译运行

```bash
cd pc
make
./demo
```
