# 07-function-pointer — 写死的函数怎么换

第 7 章 [写死的函数怎么换](../../../book/03-多态/07-写死的函数怎么换.md) 的配套代码。

## 演化点

`struct led` 加一个函数指针字段 `on_func`。`led_on()` 内部不再写死调谁，而是 `me->on_func(me)`。

```c
struct led {
	struct led_base base;
	uint8_t brightness;
	bool    is_on;
	int (*on_func)(struct led *me);    /* 新增 */
};
```

## 编译运行

```bash
cd pc
make
./demo
```

预期输出: 红灯填 `led_on_gpio_style`，蓝灯填 `led_on_pwm_style`，应用层都调 `led_on(&xxx)`。再演示运行时把 `red_led.on_func` 直接换成另一个函数，行为立刻变。
