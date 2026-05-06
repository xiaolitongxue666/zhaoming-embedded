# 08-callback — 把号码给别人拨

第 8 章 [把号码给别人拨](../../../book/03-多态/08-把号码给别人拨.md) 的配套代码。

## 演化点

函数指针不止能塞进 struct 字段（ch07），还能当函数参数：

```c
int test_led(struct led *me,
             int (*on)(struct led *me),
             int (*off)(struct led *me));
```

```c
typedef void (*led_state_cb)(struct led *me, bool new_state);
int led_register_state_cb(struct led *me, led_state_cb cb);
```

调用方把 on/off 函数指针传进来，`test_led` 内部不写死调谁。注册一个 state callback，LED 状态变化时驱动自动调一下。

## 编译运行

```bash
cd pc
make
./demo
```
