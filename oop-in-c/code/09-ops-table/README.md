# 09-ops-table — ops 操作表

第 9 章 [参数长到换行](../../../book/03-多态/09-ops操作表.md) 的配套代码。

## 演化点

引入 `struct led_ops`，把多个函数指针打包成一张表：

```c
typedef int (*led_action_fn)(struct led *me);

struct led_ops {
	led_action_fn          on;
	led_action_fn          off;
	led_action_fn          toggle;
	led_set_brightness_fn  set_brightness;
};
```

`test_led` 接受一个 `const struct led_ops *ops`，按名字访问 `ops->on / ops->off`。

ops 表常驻 `.rodata`（`const + extern`），所有同类型 LED 共享一份。

## 编译运行

```bash
cd pc
make
./demo
```
