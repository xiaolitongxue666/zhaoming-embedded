# 11-polymorphism — 多态完整图景

第 11 章 [同名函数不同行为](../../../book/03-多态/11-多态完整图景.md) 的配套代码。

## 演化点

两件事汇合到一起：

### 1. led 多态完整版

`me->ops->on(me)` 真正 dispatch。`led_on(struct led_base *me)` 单一接口走完所有子类。

```c
struct led_base *all_leds[3] = { &red.base, &blue.base, &green.base };
for (int i = 0; i < 3; ++i)
	led_on(all_leds[i]);     /* 同一行代码, 不同的实现 */
```

### 2. platform 层从函数式重构成 ops 表式

ch01-ch10 的 `platform_gpio_*(...)` 是函数式包装，编译期决定平台。

ch11 演化：

```c
struct platform_ops {
	void (*gpio_init)(uint8_t pin, uint8_t mode);
	void (*gpio_deinit)(uint8_t pin);
	void (*gpio_write)(uint8_t pin, bool value);
	bool (*gpio_read)(uint8_t pin);
};

extern const struct platform_ops *platform;
```

调用方式从 `platform_gpio_write(pin, val)` 变成 `platform->gpio_write(pin, val)`。启动时 `platform_select_pc() / platform_select_stm32()` 切换平台。

注意：本章的 `platform_ops.h` 在 `oop-in-c/code/11-polymorphism/pc/` 内部，**不动** `oop-in-c/code/common/platform.h`（那个给 ch01-ch10 共用的函数式形态保留）。第 15 章 platform 层 ops 化的高潮章会沿用这套接口。

## 编译运行

```bash
cd pc
make
./demo
```
