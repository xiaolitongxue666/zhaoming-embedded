# 10-vptr — ops 放进对象

第 10 章 [ops 放进对象](../../../book/03-多态/10-ops放进对象.md) 的配套代码。

## 演化点

`struct led_base` 加 `const struct led_ops *ops` 字段，作为第一个字段。子类 init 把对应的 `const ops` 表交给 `led_base_init`，base 一次填好。调用方拿到 `base` 指针后直接 `me->ops->on(me)`。

```c
struct led_base {
    const struct led_ops *ops;     /* 新增, 第一个字段 */
    const char           *name;
    bool                  is_on;
};
```

调用形态从

```c
test_led(&red_led.base, &led_ops_gpio);   /* ch09: 调用方传 ops */
```

变为

```c
test_led(&red_led.base);                   /* ch10: ops 在 base 自带 */
```

或直接：

```c
struct led_base *me = &red_led.base;
me->ops->on(me);
me->ops->off(me);
```

## 文件清单

```
pc/
├── Makefile           gcc -Wall -Wextra -std=c99
├── led_base.h         struct led_base 加 ops 字段
├── led_base.c         led_base_init 把 ops 一次填好
├── led.h              struct led_ops + struct led_gpio/led_pwm + test_led
├── led.c              gpio_xxx / pwm_xxx 实现 + 两张 const ops 表 + test_led
└── main.c             演示 test_led(&me.base) 与 me->ops->on(me)
stm32-snippet/
└── led_stm32.c        STM32 HAL 实现 platform_gpio_*
linux-snippet/
└── led_linux.c        Linux sysfs 实现 platform_gpio_*
```

## 编译运行

```bash
cd pc
make
./demo
```
