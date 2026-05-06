# 10-vptr — ops 放进对象

第 10 章 [ops 放进对象](../../../book/03-多态/10-ops放进对象.md) 的配套代码。

## 演化点

`struct led_base` 加 `const struct led_ops *ops` 字段，作为第一个字段（vptr 落地）。

```c
struct led_base {
	const struct led_ops *ops;     /* 新增, 第一个字段 */
	uint8_t pin;
};
```

应用层调用从

```c
test_led(&red_led, &led_ops_gpio);     /* ch09: 调用方传 ops */
```

变为

```c
led_on(&red_led.base);                  /* ch10: ops 在 base 自带 */
```

## 编译运行

```bash
cd pc
make
./demo
```
