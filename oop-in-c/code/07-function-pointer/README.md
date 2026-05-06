# 07-function-pointer — 写死的函数怎么换

第 7 章 [写死的函数怎么换](../../../book/03-多态/07-写死的函数怎么换.md) 的配套代码。

## 演化点

引入函数指针变量。一个变量 `fp` 能存某个函数的地址，需要时通过这个变量调用：

```c
void (*fp)(int);

fp = gpio_on;     /* 存号码: 函数名不带括号 = 取地址 */
fp(15);           /* 拨号: 实际调 gpio_on(15) */

fp = pwm_on;      /* 换号码 */
fp(15);           /* 这次拨通 pwm_on */
```

本章不把指针塞进任何 struct，也不引入 LED 结构。`gpio_on` / `pwm_on` / `i2c_on` 在 `main.c` 里是 printf 占位，目的是让 `fp` 跳到不同地址时跑出可见的不同输出。"塞进 struct" 这件事在 ch10 ops 字段放进 base 时正式发生。

真实硬件上 `gpio_on` 长什么样：

- STM32 HAL 实现：`stm32-snippet/led_stm32.c`
- Linux 用户态 sysfs 实现：`linux-snippet/led_linux.c`

两个 snippet 给出"`gpio_on` 在不同平台上的实际指令"，但和函数指针主线本身没关系。

## 编译运行

```bash
cd pc
make
./demo
```

预期输出节选：

```
--- fp = gpio_on; fp(15); ---
  [GPIO] pin 15 ON

--- fp = pwm_on; fp(15); ---
  [PWM] channel 15 ON (duty 100)

--- fp = i2c_on; fp(0x50); ---
  [I2C] addr 0x50 ON (cmd 0x01)
```

同一个 `fp`，存进不同函数地址，跳转目标不一样，实际跑出来的就是不同的函数。
